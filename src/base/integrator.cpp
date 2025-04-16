//
// Created by Zero on 2023/6/19.
//

#include "integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"
#include "base/sampler.h"

namespace vision {

void Integrator::invalidation() const noexcept {
    Film *film = scene().film();
    if (film->enable_accumulation()) {
        frame_index_ = 0u;
        render_time_ = 0;
    }
}

template<typename SF, typename SS>
SampledSpectrum IlluminationIntegrator::direct_lighting(const Interaction &it, const SF &sf, LightSample ls,
                                                        Bool occluded, TSampler &sampler,
                                                        const SampledWavelengths &swl, SS &ss, bool mis) {
    Float3 wi = normalize(ls.p_light - it.pos);
    ScatterEval scatter_eval = sf.evaluate(it.wo, wi, MaterialEvalMode::All,
                                           BxDFFlag::All, TransportMode::Radiance);
    ss = sf.sample(it.wo, sampler);
    Bool is_delta_light = ls.eval.pdf < 0;
    Float weight = mis ? (select(is_delta_light, 1.f, vision::MIS_weight<D>(ls.eval.pdf, scatter_eval.pdf()))) : 1.f;
    ls.eval.pdf = select(is_delta_light, -ls.eval.pdf, ls.eval.pdf);
    SampledSpectrum Ld = {swl.dimension(), 0.f};
    $if(!occluded && scatter_eval.valid() && ls.valid()) {
        Ld = ls.eval.L * scatter_eval.f * weight / ls.eval.pdf;
        frame_buffer().visualizer()->condition_add_line_segment(it.pos, ls.p_light);
    };
    return Ld;
}

#define VS_MAKE_DIRECT_LIGHTING_INSTANCE(T, U)                                                                                 \
    template SampledSpectrum IlluminationIntegrator::direct_lighting<T, U>(const Interaction &it, const T &sf, LightSample ls, \
                                                                           Bool occluded, TSampler &sampler,                   \
                                                                           const SampledWavelengths &swl, U &ss, bool mis);
VS_MAKE_DIRECT_LIGHTING_INSTANCE(PhaseFunction, PhaseSample)
VS_MAKE_DIRECT_LIGHTING_INSTANCE(MaterialEvaluator, BSDFSample)

#undef VS_MAKE_DIRECT_LIGHTING_INSTANCE

void RenderEnv::initial(TSampler &sampler, const Uint &frame_index, const TSpectrum &spectrum) noexcept {
    Uint2 pixel = dispatch_idx().xy();
    frame_index_.emplace(frame_index);
    SampledWavelengths wavelengths{spectrum->dimension()};
    sampler->temporary([&](Sampler *) {
        sampler->start(pixel, frame_index, -1);
        wavelengths = spectrum->sample_wavelength(sampler);
    });
    swl_.emplace(wavelengths);
}

IlluminationIntegrator::IlluminationIntegrator(const vision::IntegratorDesc &desc)
    : Integrator(desc),
      max_depth_(desc["max_depth"].as_uint(16)),
      min_depth_(desc["min_depth"].as_uint(5)),
      rr_threshold_(desc["rr_threshold"].as_float(1.f)),
      mis_mode_(MISMode(desc["mis_mode"].as_int(0))),
      separate_(desc["separate"].as_bool(false)),
      denoiser_(Node::create_shared<Denoiser>(desc.denoiser_desc)) {}

void IlluminationIntegrator::prepare() noexcept {
    encode_data();
    datas().reset_device_buffer_immediately(device());
    datas().register_self();
    datas().upload_immediately();
}

void IlluminationIntegrator::update_device_data() noexcept {
    if (has_changed()) {
        update_data();
        upload_immediately();
    }
}

bool IlluminationIntegrator::render_UI(ocarina::Widgets *widgets) noexcept {
    bool open = widgets->use_folding_header(
        ocarina::format("{} integrator", impl_type().data()),
        [&] {
            changed_ |= widgets->button_click("recompile", [&] {
                //                MaterialRegistry::instance().remedy();
                //                scene().fill_instances();
                //                pipeline()->update_geometry();
                compile();
            });
            changed_ |= widgets->input_uint_limit("max depth", &max_depth_.hv(), 0, 30, 1, 1);
            changed_ |= widgets->input_uint_limit("min depth", &min_depth_.hv(), 0, 20, 1, 1);
            changed_ |= widgets->drag_float("rr threshold", &rr_threshold_.hv(), 0.01, 0, 1);
            render_sub_UI(widgets);
        });
    denoiser_->render_UI(widgets);
    return open;
}

CommandList IlluminationIntegrator::denoise(RealTimeDenoiseInput &input) const noexcept {
    CommandList ret;
    if (!denoiser_) {
        return ret;
    }
    ret << denoiser_->dispatch(input);
    return ret;
}

SampledSpectrum IlluminationIntegrator::evaluate_miss(RayState &rs, const Float3 &normal,
                                                      const Float &scatter_pdf, const Uint &bounces,
                                                      const SampledWavelengths &swl) const noexcept {
    TLightSampler &light_sampler = scene().light_sampler();
    SampledSpectrum ret = spectrum()->zero();
    const Geometry &geometry = pipeline()->geometry();
    if (light_sampler->env_light()) {
        LightSampleContext p_ref;
        p_ref.pos = rs.origin();
        p_ref.ng = normal;
        SampledSpectrum tr = spectrum()->one();
        if (scene().has_medium()) {
            rs.ray.dir_max.w = scene().world_diameter();
            tr = geometry.Tr(scene(), swl, rs);
        }
        LightEval eval = light_sampler->evaluate_miss_wi(p_ref, rs.direction(), swl);
        Float weight = MIS_weight<D>(scatter_pdf, eval.pdf);
        weight = correct_bsdf_weight(weight, bounces);
        ret = eval.L * tr * weight;
    }
    return ret;
}

Float3 IlluminationIntegrator::Li(RayState rs, Float scatter_pdf, const Uint &max_depth, SampledSpectrum throughput,
                                  bool only_direct, const HitContext &hc, const RenderEnv &render_env) const noexcept {
    Pipeline *rp = pipeline();
    TSampler &sampler = scene().sampler();
    TLightSampler &light_sampler = scene().light_sampler();

    const SampledWavelengths &swl = render_env.sampled_wavelengths();
    const Geometry &geometry = rp->geometry();

    TriangleHitVar hit;
    Interaction it{scene().has_medium()};
    Float3 prev_surface_ng = rs.direction();

    Float3 ret = make_float3(0.f);

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
                scene().mediums().dispatch(rs.medium, [&](const Medium *medium) {
                    SampledSpectrum medium_throughput = medium->sample(rs, it, sampler, swl);
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
        Env::instance().set("bounces", bounces);
        comment("estimate direct lighting");
        comment("sample light");
        LightSample light_sample = light_sampler->sample_wi(it, sampler, swl);
        RayState shadow_ray;
        Bool occluded = geometry.occluded(it, light_sample.p_light, &shadow_ray);
        SampledSpectrum tr = geometry.Tr(scene(), swl, shadow_ray);
        comment("sample bsdf");
        BSDFSample bsdf_sample{swl};
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
    return ret;
}

Float3 IlluminationIntegrator::Li(vision::RayState rs, Float scatter_pdf,
                                  SampledSpectrum throughput, const HitContext &hc, const RenderEnv &render_env) const noexcept {
    return Li(rs, scatter_pdf, *max_depth_, throughput, max_depth_.hv() < 2, hc, render_env);
}

}// namespace vision