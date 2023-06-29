//
// Created by Zero on 2023/6/29.
//

#include "render_graph/pass.h"
#include "base/mgr/global.h"
#include "base/integrator.h"
#include "base/mgr/pipeline.h"

namespace vision {

class IntegratePass : public RenderPass {
private:
    Shader<void(uint, Buffer<float4>)> _shader;

public:
    explicit IntegratePass(const RenderPassDesc &desc)
        : RenderPass(desc) {
    }

    void compile() noexcept override {
        Kernel kernel = [&](Uint frame_index, BufferVar<float4> output) {
            Uint2 pixel = dispatch_idx().xy();
            sampler()->start_pixel_sample(pixel, frame_index, 0);
            SensorSample ss = sampler()->sensor_sample(pixel, camera()->filter());
            camera()->load_data();
            Float scatter_pdf = 1e16f;
            RayState rs = camera()->generate_ray(ss);
            Float3 L = integrator()->Li(rs, scatter_pdf);
            output.write(dispatch_id(), make_float4(L, 1.f));
        };
        _shader = device().compile(kernel, "integrate pass");
    }

    [[nodiscard]] Command *dispatch() noexcept override {
        return _shader(pipeline()->frame_index(),
                       res<Buffer<float4>>("radiance"))
            .dispatch(pipeline()->resolution());
    }

    [[nodiscard]] ChannelList outputs() const noexcept override {
        return {{"radiance", "radiance", false, ResourceFormat::FLOAT4}};
    }

    [[nodiscard]] static Integrator *integrator() noexcept { return scene().integrator(); }
    [[nodiscard]] static Camera *camera() noexcept { return scene().camera(); }
    [[nodiscard]] static Sampler *sampler() noexcept { return scene().sampler(); }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IntegratePass)