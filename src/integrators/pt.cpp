//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "core/render_pipeline.h"

namespace vision {
using namespace ocarina;
class PathTracingIntegrator : public Integrator {
public:
    explicit PathTracingIntegrator(const IntegratorDesc &desc) : Integrator(desc) {}

    void compile_shader(RenderPipeline *rp) noexcept override {
        Camera *camera = rp->scene().camera();
        Sampler *sampler = rp->scene().sampler();
        DeviceData &data = rp->device_data();
        Accel &accel = rp->device_data().accel;
        _kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            sampler->start_pixel_sample(pixel, frame_index, 0);
            SensorSample ss = sampler->sensor_sample(pixel);
            auto [ray, weight] = camera->generate_ray(ss);
            Var hit = accel.trace_closest(ray);
            Var p = ray->direction();
            $if(hit->is_miss()) {
                camera->film()->add_sample(pixel, make_float4(0, 0, 0, 1), 0);
                $return();
            };
            auto si = data.compute_surface_interaction(hit);
            Float3 normal = si.s_uvn.normal();
            normal = (normal + 1.f) / 2.f;
            camera->film()->add_sample(pixel, normal, frame_index);
        };
        _shader = rp->device().compile(_kernel);
    }

    void render(RenderPipeline *rp) const noexcept override {
        Stream &stream = rp->stream();
        Clock clk;
        stream << _shader(rp->frame_index()).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
        cout << clk.elapse_ms() << endl;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)