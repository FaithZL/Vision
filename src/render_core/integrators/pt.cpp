//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"

namespace vision {
using namespace ocarina;
class PathTracingIntegrator : public RayTracingIntegrator {
public:
    explicit PathTracingIntegrator(const IntegratorDesc &desc)
        : RayTracingIntegrator(desc) {}

    void prepare() noexcept override {
        RayTracingIntegrator::prepare();
        Pipeline *rp = pipeline();
        auto init_buffer = [&]<typename T>(RegistrableBuffer<T> &buffer, const string &desc = "") {
            buffer.super() = device().create_buffer<T>(rp->pixel_num(), desc);
            vector<T> vec{rp->pixel_num(), T{}};
            buffer.upload_immediately(vec.data());
            buffer.register_self();
        };
        init_buffer(_pixel_data, "RealTimeIntegrator::_pixel_data");
    }

    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void compile() noexcept override {
        Camera *camera = scene().camera().get();
        Sampler *sampler = scene().sampler();
        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Env::instance().clear_global_vars();
            Uint2 pixel = dispatch_idx().xy();
            RenderEnv render_env;
            render_env.initial(sampler, frame_index, spectrum());
            sampler->start(pixel, frame_index, 0);
            camera->load_data();
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            Float scatter_pdf = 1e16f;
            RayState rs = camera->generate_ray(ss);
            OCPixelData pixel_data;
            Env::instance().set("p_film", ss.p_film);
            Float3 L = Li(rs, scatter_pdf, spectrum().one(), {pixel_data},render_env) * ss.filter_weight;
            camera->radiance_film()->add_sample(pixel, L, frame_index);
        };
        _shader = device().compile(kernel, "path tracing integrator");
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << Env::debugger().upload();
        stream << _shader(_host_frame_index++).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
        Env::debugger().reset_range();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)