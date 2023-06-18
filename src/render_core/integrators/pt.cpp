//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"

namespace vision {
using namespace ocarina;
class PathTracingIntegrator : public Integrator {
public:
    explicit PathTracingIntegrator(const IntegratorDesc &desc)
        : Integrator(desc) {}

    template<typename SF, typename SS>
    static SampledSpectrum direct_lighting(Interaction it, const SF &sf, LightSample ls,
                                           Bool occluded, Sampler *sampler,
                                           const SampledWavelengths &swl, SS &ss) {
        Float3 wi = normalize(ls.p_light - it.pos);
        ScatterEval scatter_eval = sf.evaluate(it.wo, wi);
        ss = sf.sample(it.wo, sampler);
        Bool is_delta_light = ls.eval.pdf < 0;
        Float weight = select(is_delta_light, 1.f, mis_weight<D>(ls.eval.pdf, scatter_eval.pdf));
        ls.eval.pdf = select(is_delta_light, -ls.eval.pdf, ls.eval.pdf);
        SampledSpectrum Ld = {swl.dimension(), 0.f};
        $if(!occluded && scatter_eval.valid() && ls.valid()) {
            Ld = ls.eval.L * scatter_eval.f * weight / ls.eval.pdf;
        };
        return Ld;
    }

    void compile_shader() noexcept override {
        Pipeline *rp = pipeline();
        Camera *camera = scene().camera();
        Sampler *sampler = scene().sampler();
        LightSampler *light_sampler = scene().light_sampler();
        Spectrum &spectrum = rp->spectrum();

        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            _frame_index = frame_index;
            Bool debug = all(pixel == make_uint2(512, 512));
            sampler->start_pixel_sample(pixel, frame_index, 0);
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            RayState rs = camera->generate_ray(ss);
            Float scatter_pdf = 1e16f;
            SampledWavelengths swl = spectrum.sample_wavelength(sampler);
            SampledSpectrum Li = {swl.dimension(), 0.f};
            SampledSpectrum throughput = {swl.dimension(), 1.f};
            Geometry &geometry = rp->geometry();
            Float eta_scale = 1.f;

            $for(&bounces, 0, _max_depth) {
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
                        Li += eval.L * tr * throughput * weight;
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
                    bounces -= 1;
                    $continue;
                };

                comment("hit light");
                $if(it.has_emission()) {
                    LightSampleContext p_ref;
                    p_ref.pos = rs.origin();
                    p_ref.ng = rs.direction();
                    LightEval eval = light_sampler->evaluate_hit(p_ref, it, swl);
                    SampledSpectrum tr = geometry.Tr(scene(), swl, rs);
                    Float weight = mis_weight<D>(scatter_pdf, eval.pdf);
                    Li += eval.L * throughput * weight * tr;
                };

                comment("estimate direct lighting");
                comment("sample light");
                LightSample light_sample = light_sampler->sample(it, sampler, swl);
                RayState shadow_ray;
                Bool occluded = geometry.occluded(it, light_sample.p_light, &shadow_ray);
                SampledSpectrum tr = geometry.Tr(scene(), swl, shadow_ray);
                comment("sample bsdf");
                BSDFSample bsdf_sample{swl.dimension()};
                SampledSpectrum Ld = {swl.dimension(), 0.f};

                auto sample_surface = [&]() {
                    scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                        BSDF bsdf = material->compute_BSDF(it, swl);
                        if (auto dispersive = spectrum.is_dispersive(&bsdf)) {
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
                Li += throughput * Ld * tr;
                eta_scale *= sqr(bsdf_sample.eta);
                Float lum = throughput.max();
                $if(!bsdf_sample.valid() || lum == 0.f) {
                    $break;
                };
                throughput *= bsdf_sample.eval.value();
                $if(lum < _rr_threshold && bounces >= _min_depth) {
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
            camera->radiance_film()->add_sample(pixel, spectrum.linear_srgb(Li, swl), frame_index);
        };
        _shader = rp->device().compile(kernel, "path tracing integrator");
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << _shader(rp->frame_index()).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)