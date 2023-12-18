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

Float3 IlluminationIntegrator::Li(vision::RayState rs, Float scatter_pdf, Interaction *first_it) const noexcept {
    Pipeline *rp = pipeline();
    Sampler *sampler = scene().sampler();
    LightSampler *light_sampler = scene().light_sampler();

    SampledWavelengths swl = spectrum().sample_wavelength(sampler);
    SampledSpectrum value = {swl.dimension(), 0.f};
    SampledSpectrum throughput = {swl.dimension(), 1.f};
    SampledSpectrum curr_throughput = {swl.dimension(), 1.f};
    const Geometry &geometry = rp->geometry();

    Float eta_scale = 1.f;
    Int bounces = 0;
    $loop {
        $if(bounces >= *_max_depth) {
            $break;
        };
        Var hit = geometry.trace_closest(rs.ray);
        comment("miss");
        $if(hit->is_miss()) {
            if (light_sampler->env_light()) {
                LightSampleContext p_ref;
                p_ref.pos = rs.origin();
                p_ref.ng = rs.direction();
                SampledSpectrum tr = {swl.dimension(), 1.f};
                if (scene().has_medium()) {
                    rs.ray.dir_max.w = scene().world_diameter();
                    tr = geometry.Tr(scene(), swl, rs);
                }
                LightEval eval = light_sampler->evaluate_miss(p_ref, rs.direction(), swl);
                Float weight = mis_weight<D>(scatter_pdf, eval.pdf);
                value += eval.L * tr * throughput * weight;
            }
            $break;
        };

        Interaction it = geometry.compute_surface_interaction(hit, rs.ray);

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
            $continue;
        };

        if (first_it) {
            $if(bounces == 0) { *first_it = it; };
        }

        comment("hit light");
        $if(it.has_emission()) {
            LightSampleContext p_ref;
            p_ref.pos = rs.origin();
            p_ref.ng = rs.direction();
            LightEval eval = light_sampler->evaluate_hit(p_ref, it, swl);
            SampledSpectrum tr = geometry.Tr(scene(), swl, rs);
            Float weight = mis_weight<D>(scatter_pdf, eval.pdf);
            value += eval.L * throughput * weight * tr;
        };

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
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                BSDF bsdf = material->compute_BSDF(it, swl);
                if (auto dispersive = spectrum().is_dispersive(&bsdf)) {
                    $if(*dispersive) {
                        swl.invalidation_secondary();
                    };
                }
                Ld = direct_lighting(it, bsdf, light_sample, occluded,
                                     sampler, swl, bsdf_sample);
            });
        };

        if (scene().has_medium()) {
            $if(it.has_phase()) {
                PhaseSample ps{swl.dimension()};
                Ld = direct_lighting(it, it.phase, light_sample, occluded, sampler, swl, ps);
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
        Float lum = throughput.max();
        $if(!bsdf_sample.valid() || lum == 0.f) {
            $break;
        };
        eta_scale *= sqr(rcp(bsdf_sample.eta));
        curr_throughput = bsdf_sample.eval.value();
        throughput *= curr_throughput;
        $if(lum * eta_scale < *_rr_threshold && bounces >= *_min_depth) {
            Float q = min(0.95f, lum);
            Float rr = sampler->next_1d();
            $if(q < rr) {
                $break;
            };
            throughput /= q;
        };
        scatter_pdf = bsdf_sample.eval.pdf;
        rs = it.spawn_ray_state(bsdf_sample.wi);
        bounces += 1;
    };
    return spectrum().linear_srgb(value, swl);
}

}// namespace vision