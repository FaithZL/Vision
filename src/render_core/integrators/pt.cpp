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

    VS_MAKE_PLUGIN_NAME_FUNC

    void prepare() noexcept override {
        IlluminationIntegrator::prepare();
        denoiser_->prepare();
        frame_buffer().prepare_hit_buffer();
        frame_buffer().prepare_gbuffer();
        // albedo
        frame_buffer().prepare_bufferB();
        // emission
        frame_buffer().prepare_bufferC();
        frame_buffer().prepare_motion_vectors();
    }

    [[nodiscard]] Film *film() noexcept { return scene().film(); }
    [[nodiscard]] const Film *film() const noexcept { return scene().film(); }

    void compile() noexcept override {
        denoiser_->compile();
        film()->compile();
        Camera &camera = scene().camera();
        Sampler &sampler = scene().sampler();
        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Env::instance().clear_global_vars();
            Uint2 pixel = dispatch_idx().xy();
            RenderEnv render_env;
            render_env.initial(sampler, frame_index, spectrum());
            sampler->start(pixel, frame_index, 0);
            camera->load_data();
            load_data();
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            Float scatter_pdf = 1e16f;
            RayState rs = camera->generate_ray(ss);
            Float3 L = Li(rs, scatter_pdf, spectrum()->one(), {}, render_env) * ss.filter_weight;
            film()->rt_buffer().write(dispatch_id(), make_float4(L, 1.f));
        };
        shader_ = device().compile(kernel, "path tracing integrator");
    }

    RealTimeDenoiseInput denoise_input() const noexcept {
        RealTimeDenoiseInput ret;
        Camera &camera = scene().camera();
        ret.frame_index = frame_index_;
        ret.resolution = pipeline()->resolution();
        ret.gbuffer = frame_buffer().cur_gbuffer(frame_index_);
        ret.prev_gbuffer = frame_buffer().prev_gbuffer(frame_index_);
        ret.motion_vec = frame_buffer().motion_vectors();
        ret.radiance = film()->rt_buffer();
        ret.albedo = frame_buffer().bufferB();
        ret.emission = frame_buffer().bufferC();
        ret.output = film()->output_buffer();
        return ret;
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << Env::debugger().upload();
        stream << frame_buffer().compute_GBuffer(frame_index_,
                                                 frame_buffer().cur_gbuffer(frame_index_),
                                                 frame_buffer().motion_vectors(),
                                                 frame_buffer().bufferB(),
                                                 frame_buffer().bufferC());
        stream << shader_(frame_index_).dispatch(rp->resolution());
        RealTimeDenoiseInput input = denoise_input();
        if (film()->enable_accumulation()) {
            stream << film()->accumulate(film()->rt_buffer(), film()->accumulation_buffer(), frame_index_);
            stream << film()->tone_mapping(film()->accumulation_buffer(), film()->output_buffer());
        } else {
            stream << film()->tone_mapping(film()->rt_buffer(), film()->output_buffer());
        }
        stream << denoise(input);
        stream << film()->gamma_correct(film()->output_buffer(), film()->output_buffer());
        stream << synchronize();
        stream << commit();
        Env::debugger().reset_range();
        increase_frame_index();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)