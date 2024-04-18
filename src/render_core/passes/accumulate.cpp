//
// Created by Zero on 2023/6/29.
//

#include "render_graph/pass.h"
#include "base/mgr/pipeline.h"

namespace vision {

class AccumulatePass : public RenderPass {
private:
    Shader<void(Buffer<float4>, Buffer<float4>, uint)> shader_;

public:
    explicit AccumulatePass(const PassDesc &desc)
        : RenderPass(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    void compile() noexcept override {
        Kernel kernel = [&](BufferVar<float4> input, BufferVar<float4> output, Uint frame_index) {
            Float4 old_pixel = output.read(dispatch_id());
            Float4 new_pixel = lerp(make_float4(1.f / cast<float>(frame_index + 1.f)), old_pixel, input.read(dispatch_id()));
            new_pixel.w = 1.f;
            output.write(dispatch_id(), new_pixel);
        };
        shader_ = device().compile(kernel, "accumulate");
    }

    [[nodiscard]] ChannelList inputs() const noexcept override {
        return {
            {"input", "input", false, ResourceFormat::FLOAT4}};
    }

    [[nodiscard]] ChannelList outputs() const noexcept override {
        return {
            {"output", "output", false, ResourceFormat::FLOAT4}};
    }

    [[nodiscard]] Command *dispatch() noexcept override {
        return shader_(res<Buffer<float4>>("input"),
                       res<Buffer<float4>>("output"),
                       pipeline()->frame_index())
            .dispatch(pipeline()->resolution());
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AccumulatePass)