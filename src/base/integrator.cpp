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
    Film *film = scene().radiance_film();
    if (film->enable_accumulation()) {
        _frame_index = 0u;
        _render_time = 0;
    }
}

void RenderEnv::initial(Sampler *sampler, const Uint &frame_index, const Spectrum &spectrum) noexcept {
    Uint2 pixel = dispatch_idx().xy();
    _frame_index.emplace(frame_index);
    SampledWavelengths wavelengths{spectrum.dimension()};
    sampler->temporary([&](Sampler *) {
        sampler->start(pixel, frame_index, -1);
        wavelengths = spectrum.sample_wavelength(sampler);
    });
    _swl.emplace(wavelengths);
}

IlluminationIntegrator::IlluminationIntegrator(const vision::IntegratorDesc &desc)
    : Integrator(desc),
      _max_depth(desc["max_depth"].as_uint(16)),
      _min_depth(desc["min_depth"].as_uint(5)),
      _rr_threshold(desc["rr_threshold"].as_float(1.f)),
      _mis_mode(MISMode(desc["mis_mode"].as_int(0))),
      _separate(desc["separate"].as_bool(false)),
      _denoiser(scene().load<Denoiser>(desc.denoiser_desc)) {}

void IlluminationIntegrator::prepare() noexcept {
    encode_data();
    datas().reset_device_buffer_immediately(device());
    datas().register_self();
    datas().upload_immediately();
    _denoiser->prepare();
    Pipeline *rp = pipeline();
    if (!_denoiser) {
        return;
    }
}

CommandList IlluminationIntegrator::denoise() const noexcept {
    CommandList ret;
    if (!_denoiser) {
        return ret;
    }
    vision::DenoiseInput input;
    input.frame_index = _frame_index;
    input.resolution = pipeline()->resolution();
    input.radiance = &(scene().camera()->radiance_film()->original_buffer());
    input.gpu_output = &(scene().camera()->radiance_film()->denoised_buffer());
    ret << _denoiser->dispatch(input);
    return ret;
}

Float2 IlluminationIntegrator::compute_motion_vec(const Float2 &p_film, const Float3 &cur_pos, const Bool &is_hit) noexcept {
    Camera *camera = scene().camera().get();
    Float2 ret = make_float2(0.f);
    $if(is_hit) {
        Float2 raster_coord = camera->prev_raster_coord(cur_pos).xy();
        ret = p_film - raster_coord;
    };
    return ret;
}

SampledSpectrum IlluminationIntegrator::evaluate_miss(RayState &rs, const Float3 &normal,
                                                      const Float &scatter_pdf, const Uint &bounces,
                                                      const SampledWavelengths &swl) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    SampledSpectrum ret = spectrum().zero();
    const Geometry &geometry = pipeline()->geometry();
    if (light_sampler->env_light()) {
        LightSampleContext p_ref;
        p_ref.pos = rs.origin();
        p_ref.ng = normal;
        SampledSpectrum tr = spectrum().one();
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
    Sampler *sampler = scene().sampler();
    LightSampler *light_sampler = scene().light_sampler();

    const SampledWavelengths &swl = render_env.sampled_wavelengths();
    SampledSpectrum value = spectrum().zero();
    const Geometry &geometry = rp->geometry();

    OCHit hit;
    Interaction it{scene().has_medium()};
    Float3 prev_surface_ng = rs.direction();

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
            value += evaluate_miss(rs, prev_surface_ng, scatter_pdf, bounces, swl) * throughput;
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
            value += eval.L * throughput * weight * tr;
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
                albedo = spectrum().linear_srgb(evaluator.albedo(), swl);
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

        value += throughput * Ld * tr;
        eta_scale *= sqr(bsdf_sample.eta);
        Float lum = throughput.max();
        $if(!bsdf_sample.valid() || lum == 0.f) {
            $break;
        };
        throughput *= bsdf_sample.eval.throughput();
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

    if (only_direct && _mis_mode == MISMode::EBoth) {
        /// Supplement only direct light BSDF sampling
        $for(&bounce, 1u) {
            mis_bsdf(bounce, false);
        };
    }
    return spectrum().linear_srgb(value, swl);
}

Float3 IlluminationIntegrator::Li(vision::RayState rs, Float scatter_pdf,
                                  SampledSpectrum throughput, const HitContext &hc, const RenderEnv &render_env) const noexcept {
    return Li(rs, scatter_pdf, *_max_depth, throughput, _max_depth.hv() < 2, hc, render_env);
}

BufferMgr::BufferMgr()
    : _bufferA{Global::instance().pipeline()->bindless_array()},
      _bufferB{Global::instance().pipeline()->bindless_array()} {}

}// namespace vision