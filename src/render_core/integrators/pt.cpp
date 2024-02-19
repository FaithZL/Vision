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

    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }

    void prepare() noexcept override {
        IlluminationIntegrator::prepare();
        _denoiser->prepare();
        frame_buffer().prepare_gbuffer();
        // radiance
        frame_buffer().prepare_bufferA();
        // albedo
        frame_buffer().prepare_bufferB();
        // emission
        frame_buffer().prepare_bufferC();
        frame_buffer().prepare_motion_vectors();
    }

    void compile() noexcept override {
        _denoiser->compile();
        frame_buffer().compile();
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
            Float3 L = Li(rs, scatter_pdf, spectrum().one(), {}, render_env) * ss.filter_weight;
            frame_buffer().bufferA().write(dispatch_id(), make_float4(L, 1.f));
            camera->radiance_film()->add_sample(pixel, L, frame_index);
        };
        _shader = device().compile(kernel, "path tracing integrator");
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << Env::debugger().upload();
        stream << frame_buffer().compute_GBuffer(_frame_index,
                                                 frame_buffer().gbuffer(),
                                                 frame_buffer().bufferB(),
                                                 frame_buffer().bufferC());
        stream << _shader(_frame_index).dispatch(rp->resolution());

//        stream << denoise();
        stream << synchronize();
        stream << commit();
        Env::debugger().reset_range();
        increase_frame_index();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)