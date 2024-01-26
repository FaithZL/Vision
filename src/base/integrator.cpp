//
// Created by Zero on 2023/6/19.
//

#include "integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"

namespace vision {

void Integrator::invalidation() const noexcept {
    Film *film = scene().radiance_film();
    if (film->enable_accumulation()) {
        _frame_index = 0u;
        _render_time = 0;
    }
}

Float3 IlluminationIntegrator::Li(RayState rs, Float scatter_pdf, const Uint &max_depth,
                                  bool only_direct, Interaction *first_it) const noexcept {
    Pipeline *rp = pipeline();
    Sampler *sampler = scene().sampler();
    LightSampler *light_sampler = scene().light_sampler();

    SampledWavelengths swl = spectrum().sample_wavelength(sampler);
    SampledSpectrum value = {swl.dimension(), 0.f};
    SampledSpectrum throughput = {swl.dimension(), 1.f};
    const Geometry &geometry = rp->geometry();

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
                return direct_lighting(OC_FORWARD(args)...) * 0.f;
            }
            case MISMode::ELight: {
                return direct_lighting(OC_FORWARD(args)..., false);
            }
            default: break;
        }
        return direct_lighting(OC_FORWARD(args)...);
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

    if (only_direct && _mis_mode == MISMode::EBoth) {
        /// Supplement only direct light BSDF sampling
        $for(&bounce, 1u) {
            mis_bsdf(bounce, false);
        };
    }

    return spectrum().linear_srgb(value, swl);
}

Float3 IlluminationIntegrator::Li(vision::RayState rs, Float scatter_pdf, Interaction *first_it) const noexcept {
    return Li(rs, scatter_pdf, *_max_depth, _max_depth.hv() < 2,first_it);
}

BufferMgr::BufferMgr()
    : _motion_vectors{Global::instance().pipeline()->bindless_array()},
      _direct_light{Global::instance().pipeline()->bindless_array()},
      _indirect_light{Global::instance().pipeline()->bindless_array()},
      _surfaces{Global::instance().pipeline()->bindless_array()},
      _hit_contexts{Global::instance().pipeline()->bindless_array()} {}

}// namespace vision