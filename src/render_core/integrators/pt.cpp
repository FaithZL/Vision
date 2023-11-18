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

    void compile() noexcept override {
        Camera *camera = scene().camera().get();
        Sampler *sampler = scene().sampler();
        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            sampler->start_pixel_sample(pixel, frame_index, 0);
            camera->load_data();
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            Float scatter_pdf = 1e16f;
            RayState rs = camera->generate_ray(ss);
            Float3 L = Li(rs, scatter_pdf, nullptr) * ss.filter_weight;
            Debugger::instance().execute([&] {
                Printer::instance().info("{} {}  ---------------", dispatch_idx().xy());
            });
            camera->radiance_film()->add_sample(pixel, L, frame_index);
        };
        _shader = device().compile(kernel, "path tracing integrator");
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << Debugger::instance().upload();
        stream << _shader(_frame_index++).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
        Debugger::instance().reset_range();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)