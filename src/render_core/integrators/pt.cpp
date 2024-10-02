//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"

namespace vision {
using namespace ocarina;
class PathTracingIntegrator : public IlluminationIntegrator {
public:
    PathTracingIntegrator() = default;
    explicit PathTracingIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc) {}

    VS_MAKE_PLUGIN_NAME_FUNC

    void prepare() noexcept override {
        IlluminationIntegrator::prepare();
        frame_buffer().prepare_hit_buffer();
        frame_buffer().prepare_gbuffer();
        frame_buffer().prepare_motion_vectors();
    }
    [[nodiscard]] Film *film() noexcept { return scene().film(); }
    [[nodiscard]] const Film *film() const noexcept { return scene().film(); }

    void compile() noexcept override {
        TCamera &camera = scene().camera();
        TSampler &sampler = scene().sampler();
        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Env::instance().clear_global_vars();
            Uint2 pixel = dispatch_idx().xy();
            RenderEnv render_env;
            sampler->load_data();
            camera->load_data();
            load_data();
            render_env.initial(sampler, frame_index, spectrum());
            sampler->start(pixel, frame_index, 0);
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            Float scatter_pdf = 1e16f;
            RayState rs = camera->generate_ray(ss);
//            Float3 L = Li(rs, scatter_pdf, spectrum()->one(), {}, render_env) * ss.filter_weight;
            Float3 L = Li(rs, scatter_pdf,*max_depth_, spectrum()->one(),max_depth_.hv() < 2, {}, render_env) * ss.filter_weight;
            film()->add_sample(dispatch_idx().xy(), make_float4(L, 1.f), frame_index);
        };
        shader_ = device().compile(kernel, "path tracing integrator");
    }

    Float3 Li(RayState rs, Float scatter_pdf, const Uint &max_depth, SampledSpectrum throughput,
              bool only_direct, const HitContext &hc, const RenderEnv &render_env) const noexcept override {
        Pipeline *rp = pipeline();
        TSampler &sampler = scene().sampler();
        TLightSampler &light_sampler = scene().light_sampler();

        const SampledWavelengths &swl = render_env.sampled_wavelengths();
        const Geometry &geometry = rp->geometry();
        Float3 ret = make_float3(0.f);
        auto func = [&] {
            sampler->start(dispatch_idx().xy(), render_env.frame_index(), 3);
            HitVar hit;
            Interaction it{scene().has_medium()};
            Float3 prev_surface_ng = rs.direction();



            Float3 primary_dir = rs.direction();
            auto mis_bsdf = [&](auto &bounces, bool inner) {
                hit = geometry.trace_closest(rs.ray);
                comment("miss");
                if (!inner) {
                    Bool primary_miss = all(rs.direction() == primary_dir);
                    $if(primary_miss) {
                        $break;
                    };
                }

                $if(hit->is_miss()) {
                    SampledSpectrum d = evaluate_miss(rs, prev_surface_ng, scatter_pdf, bounces, swl) * throughput;
                    ret += spectrum()->linear_srgb(d, swl);
                    $break;
                };

                it = geometry.compute_surface_interaction(hit, rs.ray);
                //        $if(bounces == 0) {
                frame_buffer().visualizer()->condition_add_line_segment(rs.origin(), it.pos);
                //        };

                if (scene().has_medium()) {
                    $if(rs.in_medium()) {
                        scene().mediums().dispatch_instance(rs.medium, [&](const Medium *medium) {
                            SampledSpectrum medium_throughput = medium->sample(rs.ray, it, swl, sampler);
                            throughput *= medium_throughput;
                        });
                    };
                }

                $if(!it.has_material() && !it.has_phase()) {
                    //todo remove no material mesh in non volumetric scene
                    comment("process no material interaction for volumetric rendering");
                    rs = it.spawn_ray_state(rs.direction());
                    bounces -= 1;
                    $continue;
                };

                if (hc.it) {
                    $if(bounces == 0) { *hc.it = it; };
                }

                comment("hit light");
                $if(it.has_emission()) {
                    LightSampleContext p_ref;
                    p_ref.pos = rs.origin();
                    p_ref.ng = prev_surface_ng;
                    LightEval eval = light_sampler->evaluate_hit_wi(p_ref, it, swl);
                    SampledSpectrum tr = geometry.Tr(scene(), swl, rs);
                    Float weight = MIS_weight<D>(scatter_pdf, eval.pdf);
                    weight = correct_bsdf_weight(weight, bounces);
                    SampledSpectrum d = eval.L * throughput * weight * tr;
                    ret += spectrum()->linear_srgb(d, swl);
                };
                prev_surface_ng = it.ng;
            };

            Float eta_scale = 1.f;
            $for(&bounces, 0, max_depth) {
                mis_bsdf(bounces, true);

                comment("estimate direct lighting");
                comment("sample light");
                LightSample light_sample = light_sampler->sample_wi(it, sampler, swl);
                RayState shadow_ray;
                Bool occluded = geometry.occluded(it, light_sample.p_light, &shadow_ray);
                SampledSpectrum tr = geometry.Tr(scene(), swl, shadow_ray);
                comment("sample bsdf");
                BSDFSample bsdf_sample{swl.dimension()};
                SampledSpectrum Ld = {swl.dimension(), 0.f};

                Float3 albedo = make_float3(0.f);

                auto sample_surface = [&]() {
                    if (separate_) {
                        MaterialEvaluator evaluator(it, swl);
                        scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                            material->build_evaluator(evaluator, it, swl);
                            swl.check_dispersive(spectrum(), evaluator);
                        });
                        Ld = direct_light_mis(it, evaluator, light_sample, occluded,
                                              sampler, swl, bsdf_sample);
                        return;
                    }
                    scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                        MaterialEvaluator evaluator = material->create_evaluator(it, swl);
                        swl.check_dispersive(spectrum(), evaluator);
                        Ld = direct_light_mis(it, evaluator, light_sample, occluded,
                                              sampler, swl, bsdf_sample);
                        albedo = spectrum()->linear_srgb(evaluator.albedo(it.wo), swl);
                    });
                };

                if (scene().has_medium()) {
                    $if(it.has_phase()) {
                        PhaseSample ps{swl.dimension()};
                        Ld = direct_light_mis(it, it.phase(), light_sample, occluded, sampler, swl, ps);
                        bsdf_sample.eval = ps.eval;
                        bsdf_sample.wi = ps.wi;
                    }
                    $else {
                        sample_surface();
                    };
                } else {
                    sample_surface();
                }
                SampledSpectrum d = throughput * Ld * tr;
                ret += spectrum()->linear_srgb(d, swl);
                eta_scale *= sqr(bsdf_sample.eta);
                Float lum = throughput.max();
                $if(!bsdf_sample.valid() || lum == 0.f) {
                    $break;
                };
                throughput *= bsdf_sample.eval.throughput();
                $if(eta_scale * lum < *rr_threshold_ && bounces >= *min_depth_) {
                    Float q = min(0.95f, lum);
                    Float rr = sampler->next_1d();
                    $if(q < rr) {
                        $break;
                    };
                    throughput /= q;
                };
                scatter_pdf = bsdf_sample.eval.pdf();
                rs = it.spawn_ray_state(bsdf_sample.wi);
            };

            if (only_direct && mis_mode_ == MISMode::EBoth) {
                /// Supplement only direct light BSDF sampling
                $for(&bounce, 1u) {
                    mis_bsdf(bounce, false);
                };
            }
        };
        func();
        return ret;
    }

    RealTimeDenoiseInput denoise_input() const noexcept {
        RealTimeDenoiseInput ret;
        TCamera &camera = scene().camera();
        ret.frame_index = frame_index_;
        ret.resolution = pipeline()->resolution();
        ret.gbuffer = frame_buffer().cur_gbuffer(frame_index_);
        ret.prev_gbuffer = frame_buffer().prev_gbuffer(frame_index_);
        ret.motion_vec = frame_buffer().motion_vectors();
        ret.radiance = film()->rt_buffer();
        ret.output = film()->output_buffer();
        return ret;
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << shader_(frame_index_).dispatch(rp->resolution());
        RealTimeDenoiseInput input = denoise_input();
        increase_frame_index();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, PathTracingIntegrator)
VS_REGISTER_CURRENT_PATH(0, "vision-integrator-pt.dll")
