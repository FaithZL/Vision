//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "core/render_pipeline.h"
#include "math/warp.h"

namespace vision {
using namespace ocarina;
class PathTracingIntegrator : public Integrator {
public:
    explicit PathTracingIntegrator(const IntegratorDesc &desc)
        : Integrator(desc) {
        _mis_weight = mis_weight<D>;
    }

    void compile_shader(RenderPipeline *rp) noexcept override {
        Camera *camera = rp->scene().camera();
        Sampler *sampler = rp->scene().sampler();
        LightSampler *light_sampler = rp->scene().light_sampler();
        DeviceData &data = rp->device_data();
        Accel &accel = rp->device_data().accel;

        _kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            sampler->start_pixel_sample(pixel, frame_index, 0);
            SensorSample ss = sampler->sensor_sample(pixel);
            RaySample rs = camera->generate_ray(ss);
            Var ray = rs.ray;
            Float bsdf_pdf = eval(1e16f);
            Float3 Li = make_float3(0.f);
            Float3 throughput = make_float3(1.f);
            $for(bounces, 0, _max_depth) {
                Var hit = accel.trace_closest(ray);
                comment("miss");
                $if(hit->is_miss()) {
                    $break;
                };
                auto si = data.compute_surface_interaction(hit);
                si.wo = normalize(-ray->direction());

                comment("hit light");
                $if(si.has_emission()) {
                    LightSampleContext p_ref;
                    p_ref.pos = ray->origin();
                    p_ref.ng = ray->direction();
                    Evaluation eval = light_sampler->evaluate_hit(p_ref, si);
                    Li += eval.f * throughput * _mis_weight(bsdf_pdf, eval.pdf);
                };

                comment("estimate direct lighting");
                comment("sample light");
                LightSample light_sample = light_sampler->sample(si, sampler->next_1d(), sampler->next_2d());
                Bool occluded = accel.trace_any(si.spawn_ray(light_sample.wi * light_sample.distance*0.99f));
                comment("sample bsdf");
                BSDFSample bsdf_sample;
                rp->dispatch<Material>(si.mat_id, rp->scene().materials(),
                                       [&](const Material *material) {
                    UP<BSDF> bsdf = material->get_BSDF(si);
                    Evaluation bsdf_eval = bsdf->evaluate(si.wo, light_sample.wi, BxDFFlag::All);

                    $if( bsdf_eval.valid()) {
                        Li += throughput * light_sample.eval.f * bsdf_eval.f * _mis_weight(light_sample.eval.pdf, bsdf_eval.pdf)
                              / light_sample.eval.pdf;
                    } $else {
                        print("wori");
                    };

                    bsdf_sample = bsdf->sample(si.wo, sampler->next_1d(), sampler->next_2d(), BxDFFlag::All);
//                    $if(!bsdf_sample.valid()) {
//                        print("{},{},{}", si.pos.x, si.pos.y, bsdf_sample.eval.pdf);
//                    };
                });
                throughput *= bsdf_sample.eval.f / bsdf_sample.eval.pdf;

                Float rr = sampler->next_1d();
                Float mp = max_comp(throughput);
                $if(mp < _rr_threshold) {
                    Float q = min(0.95f, mp);
                    $if(q < rr) {
                        $break;
                    };
                    throughput /= q;
                };
                ray = si.spawn_ray(bsdf_sample.wi);
            };

//            Var hit = accel.trace_closest(ray);
//            $if(hit->is_miss()) {
//                camera->film()->add_sample(pixel, make_float3(0), 0);
//                $return();
//            };
//            auto si = data.compute_surface_interaction(hit);
//            Float3 normal = si.s_uvn.normal();
//            normal = (normal + 1.f) / 2.f;
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