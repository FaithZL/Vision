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
    explicit PathTracingIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc) {}

    template<typename SF, typename SS>
    static SampledSpectrum direct_lighting_(const Interaction &it, const SF &sf, LightSample ls,
                                           Bool occluded, Sampler *sampler,
                                           const SampledWavelengths &swl, SS &ss, bool mis = true) {
        Float3 wi = normalize(ls.p_light - it.pos);
        ScatterEval scatter_eval = sf.evaluate(it.wo, wi);
        ss = sf.sample(it.wo, sampler);
        Bool is_delta_light = ls.eval.pdf < 0;
        Float weight = mis ? (select(is_delta_light, 1.f, vision::MIS_weight<D>(ls.eval.pdf, scatter_eval.pdf))) : 1.f;
        ls.eval.pdf = select(is_delta_light, -ls.eval.pdf, ls.eval.pdf);
        SampledSpectrum Ld = {swl.dimension(), 0.f};
        $if(!occluded && scatter_eval.valid() && ls.valid()) {
            Ld = ls.eval.L * scatter_eval.f * weight / ls.eval.pdf;
        };
//        $info_if(Ld.max() > 5000, "{} {}      {} {} {}     {} ", dispatch_idx().xy(), Ld.vec3(), vision::MIS_weight<D>(ls.eval.pdf, scatter_eval.pdf));
        return Ld;
    }

    [[nodiscard]] Float3 Li(vision::RayState rs, Float scatter_pdf, Interaction *first_it) const noexcept override{
        Pipeline *rp = pipeline();
        Sampler *sampler = scene().sampler();
        LightSampler *light_sampler = scene().light_sampler();

        SampledWavelengths swl = spectrum().sample_wavelength(sampler);
        SampledSpectrum value = {swl.dimension(), 0.f};
        SampledSpectrum throughput = {swl.dimension(), 1.f};
        const Geometry &geometry = rp->geometry();

        bool only_direct = _max_depth.hv() < 2;

        OCHit hit;
        Interaction it;
        Float3 prev_surface_ng = rs.direction();

        auto correct_bsdf_weight = [this](Float weight, Uint bounce) {
            switch (_mis_mode) {
                case MISMode::EBSDF: {
                    weight = 1.f;
                    break;
                }
                case MISMode::ELight: {
                    weight = ocarina::select(bounce == 0, weight, 0.f);
                    break;
                }
                default: break;
            }
            return weight;
        };

        auto direct_light_mis = [this]<typename... Args>(Args &&...args) -> SampledSpectrum {
            switch (_mis_mode) {
                case MISMode::EBSDF: {
                    return direct_lighting_(OC_FORWARD(args)...) * 0.f;
                }
                case MISMode::ELight: {
                    auto Ld = direct_lighting_(OC_FORWARD(args)..., false);
//                    $info_if(Ld.max() > 5000, "{} {}      {} {} {}      ", dispatch_idx().xy(), Ld.vec3());
                    return Ld;
                }
                default: break;
            }
            return direct_lighting_(OC_FORWARD(args)...);
        };
        Float3 primary_dir = rs.direction();
        auto mis_bsdf = [&](Uint &bounces, bool inner) {
            hit = geometry.trace_closest(rs.ray);
            comment("miss");
            if (!inner) {
                Bool primary_miss = all(rs.direction() == primary_dir);
                $if(primary_miss) {
                    $break;
                };
            }

            $if(hit->is_miss()) {
                if (light_sampler->env_light()) {
                    LightSampleContext p_ref;
                    p_ref.pos = rs.origin();
                    p_ref.ng = prev_surface_ng;
                    SampledSpectrum tr = {swl.dimension(), 1.f};
                    if (scene().has_medium()) {
                        rs.ray.dir_max.w = scene().world_diameter();
                        tr = geometry.Tr(scene(), swl, rs);
                    }
                    LightEval eval = light_sampler->evaluate_miss_wi(p_ref, rs.direction(), swl);
                    Float weight = MIS_weight<D>(scatter_pdf, eval.pdf);
                    weight = correct_bsdf_weight(weight, bounces);
                    value += eval.L * tr * throughput * weight;
                }
                $break;
            };

            it = geometry.compute_surface_interaction(hit, rs.ray);

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

            if (first_it) {
                $if(bounces == 0) { *first_it = it; };
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
                value += eval.L * throughput * weight * tr;
            };
            prev_surface_ng = it.ng;
        };

        Float eta_scale = 1.f;
        $for(&bounces, 0, *_max_depth) {
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

            auto sample_surface = [&]() {
                if (_separate) {
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
                });
            };

            if (scene().has_medium()) {
                $if(it.has_phase()) {
                    PhaseSample ps{swl.dimension()};
                    Ld = direct_light_mis(it, it.phase, light_sample, occluded, sampler, swl, ps);
                    bsdf_sample.eval = ps.eval;
                    bsdf_sample.wi = ps.wi;
                }
                $else {
                    sample_surface();
                };
            } else {
                sample_surface();
            }
//            $info_if(Ld.max() > 50000 , "{} {}      {} {} {}    {} {} {}     {} {} {}     {}     ", dispatch_idx().xy(), Ld.vec3(), light_sample.p_light- it.pos,throughput.vec3(), it.has_emission().cast<int>());
            value += throughput * Ld * tr;
            eta_scale *= sqr(bsdf_sample.eta);
            Float lum = throughput.max();
            $if(!bsdf_sample.valid() || lum == 0.f) {
                $break;
            };
            throughput *= bsdf_sample.eval.value();
            $if(eta_scale * lum < *_rr_threshold && bounces >= *_min_depth) {
                Float q = min(0.95f, lum);
                Float rr = sampler->next_1d();
                $if(q < rr) {
                    $break;
                };
                throughput /= q;
            };
            scatter_pdf = bsdf_sample.eval.pdf;
            rs = it.spawn_ray_state(bsdf_sample.wi);
        };

        if (only_direct) {
            /// Supplement only direct light BSDF sampling
            $for(&bounce, 1u) {
                mis_bsdf(bounce, false);
            };
        }

        return spectrum().linear_srgb(value, swl);
    }

    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void compile() noexcept override {
        Camera *camera = scene().camera().get();
        Sampler *sampler = scene().sampler();
        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            sampler->start(pixel, frame_index, 0);
            camera->load_data();
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            Float scatter_pdf = 1e16f;
            RayState rs = camera->generate_ray(ss);
            Float3 L = Li(rs, scatter_pdf, nullptr) * ss.filter_weight;
            camera->radiance_film()->add_sample(pixel, L, frame_index);
        };
        _shader = device().compile(kernel, "path tracing integrator");
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << Env::debugger().upload();
        stream << _shader(_frame_index++).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
        Env::debugger().reset_range();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)