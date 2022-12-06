//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "base/render_pipeline.h"
#include "math/warp.h"

namespace vision {
using namespace ocarina;
class PathTracingIntegrator : public Integrator {
public:
    explicit PathTracingIntegrator(const IntegratorDesc &desc)
        : Integrator(desc) {}

    void compile_shader(RenderPipeline *rp) noexcept override {
        Camera *camera = _scene->camera();
        Sampler *sampler = _scene->sampler();
        LightSampler *light_sampler = _scene->light_sampler();

        _kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            Bool debug = all(pixel == make_uint2(508, 66));
            sampler->start_pixel_sample(pixel, frame_index, 0);
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            RayState rs = camera->generate_ray(ss);
            Float bsdf_pdf = eval(1e16f);
            Float3 Li = make_float3(0.f);
            Float3 throughput = make_float3(1.f);
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
                        LightEval eval = light_sampler->evaluate_miss(p_ref, rs.direction());
                        Float weight = mis_weight<D>(bsdf_pdf, eval.pdf);
                        Li += eval.L * throughput * weight;
                    }
                    $break;
                };

                Float3 medium_throughput = make_float3(1.f);
                Interaction it = geometry.compute_surface_interaction(hit, rs.ray);
                $if(rs.in_medium()) {
                    _scene->mediums().dispatch(rs.medium, [&](const Medium *medium) {
                        auto data = medium->sample(rs.ray, sampler);
                        medium_throughput = data.first;
                        Interaction mi = data.second;
//                        $if(mi.has_phase()) {
////                            it = mi;
//
//                        };
                    });
                };

                comment("process medium interaction");


                $if(!it.has_material()) {
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
                    LightEval eval = light_sampler->evaluate_hit(p_ref, it);
                    Float weight = mis_weight<D>(bsdf_pdf, eval.pdf);
                    Li += eval.L * throughput * weight;
                };

                comment("estimate direct lighting");
                comment("sample light");
                LightSample light_sample = light_sampler->sample(it, sampler->next_1d(), sampler->next_2d());
                OCRay shadow_ray = it.spawn_ray_to(light_sample.p_light);
                Bool occluded = geometry.trace_any(shadow_ray);

                comment("sample bsdf");
                BSDFSample bsdf_sample;
                _scene->materials().dispatch(it.mat_id, [&](const Material *material) {
                    UP<BSDF> bsdf = material->get_BSDF(it);

                    ScatterEval scatter_eval;
                    Float3 wi = normalize(light_sample.p_light - it.pos);
                    scatter_eval = bsdf->evaluate(wi, BxDFFlag::All);
                    bsdf_sample = bsdf->sample(sampler->next_1d(), sampler->next_2d(), BxDFFlag::All);
                    Float3 w = bsdf_sample.wi;
                    Float3 f = bsdf_sample.eval.f;
                    //todo trick delta light
                    Bool is_delta_light = light_sample.eval.pdf < 0;
                    Float weight = select(is_delta_light, 1.f, mis_weight<D>(light_sample.eval.pdf, scatter_eval.pdf));
                    light_sample.eval.pdf = select(is_delta_light, -light_sample.eval.pdf, light_sample.eval.pdf);
                    $if(!occluded && scatter_eval.valid() && light_sample.valid()) {
                        Float3 Ld = light_sample.eval.L * scatter_eval.f * weight / light_sample.eval.pdf;
                        Li += throughput * Ld;
                    };
                });
                eta_scale *= sqr(bsdf_sample.eta);
                Float lum = luminance(throughput * eta_scale);
                $if(!bsdf_sample.valid() || lum == 0.f) {
                    $break;
                };
                throughput *= bsdf_sample.eval.f / bsdf_sample.eval.pdf;
                $if(lum < _rr_threshold && bounces >= _min_depth) {
                    Float q = min(0.95f, lum);
                    Float rr = sampler->next_1d();
                    $if(q < rr) {
                        $break;
                    };
                    throughput /= q;
                };
                bsdf_pdf = bsdf_sample.eval.pdf;
                rs = it.spawn_ray_state(bsdf_sample.wi);
            };
            camera->film()->add_sample(pixel, Li, frame_index);
        };
        _shader = rp->device().compile(_kernel);
    }

    void render(RenderPipeline *rp) const noexcept override {
        Stream &stream = rp->stream();
        stream << _shader(rp->frame_index()).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)