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
        frame_buffer().prepare_hit_buffer();
        frame_buffer().prepare_gbuffer();
        frame_buffer().prepare_motion_vectors();
    }

    [[nodiscard]] Film *film() noexcept { return scene().film(); }
    [[nodiscard]] const Film *film() const noexcept { return scene().film(); }

    void compile() noexcept override {
        TCamera &camera = scene().camera();
        TSampler &sampler = scene().sampler();
        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Env::instance().clear_global_vars();
            Uint2 pixel = dispatch_idx().xy();
            RenderEnv render_env;
            sampler->load_data();
            camera->load_data();
            load_data();
            render_env.initial(sampler, frame_index, spectrum());
            sampler->start(pixel, frame_index, 0);
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            Float scatter_pdf = 1e16f;
            RayState rs = camera->generate_ray(ss);
            Float3 L = Li(rs, scatter_pdf, spectrum()->one(), {}, render_env) * ss.filter_weight;
            film()->add_sample(dispatch_idx().xy(), make_float4(L, 1.f), frame_index);
        };
        shader_ = device().compile(kernel, "path tracing integrator");
    }

    RealTimeDenoiseInput denoise_input() const noexcept {
        RealTimeDenoiseInput ret;
        TCamera &camera = scene().camera();
        ret.frame_index = frame_index_;
        ret.resolution = pipeline()->resolution();
        ret.gbuffer = frame_buffer().cur_gbuffer(frame_index_);
        ret.prev_gbuffer = frame_buffer().prev_gbuffer(frame_index_);
        ret.motion_vec = frame_buffer().motion_vectors();
        ret.radiance = film()->rt_buffer();
        ret.output = film()->output_buffer();
        return ret;
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << shader_(frame_index_).dispatch(rp->resolution());
        RealTimeDenoiseInput input = denoise_input();
        increase_frame_index();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)