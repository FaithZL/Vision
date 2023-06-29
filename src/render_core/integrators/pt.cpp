//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"

namespace vision {
using namespace ocarina;
class PathTracingIntegrator : public UnidirectionalPathTracing {
public:
    explicit PathTracingIntegrator(const IntegratorDesc &desc)
        : UnidirectionalPathTracing(desc) {}

    void compile_shader() noexcept override {
        Camera *camera = scene().camera();
        Sampler *sampler = scene().sampler();
        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            _frame_index.emplace(frame_index);
            sampler->start_pixel_sample(pixel, frame_index, 0);
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            camera->load_data();
            Float scatter_pdf = 1e16f;
            RayState rs = camera->generate_ray(ss);
            camera->radiance_film()->add_sample(pixel, Li(rs, scatter_pdf), frame_index);
        };
        _shader = device().compile(kernel, "path tracing integrator");
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