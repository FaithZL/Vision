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

    [[nodiscard]] static Integrator *integrator() noexcept {
        return scene().integrator();
    }

    [[nodiscard]] static Camera *camera() noexcept {
        return scene().camera();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IntegratePass)