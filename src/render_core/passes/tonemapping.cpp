//
// Created by Zero on 2023/6/29.
//

#include "render_graph/pass.h"
#include "base/sensor/tonemapper.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"

namespace vision {

class ToneMappingPass : public RenderPass {
private:
    mutable SP<ToneMapper> tone_mapper_{};
    Shader<void(Buffer<float4>, Buffer<float4>)> shader_;

public:
    explicit ToneMappingPass(const PassDesc &desc)
        : RenderPass(desc) {
        ToneMapperDesc tone_mapper_desc;
        tone_mapper_desc.init(DataWrap::object());
        tone_mapper_ = Node::create_shared<ToneMapper>(tone_mapper_desc);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void compile() noexcept override {
        Kernel kernel = [&](BufferVar<float4> input, BufferVar<float4> output) {
            Float4 pixel = input.read(dispatch_id());
            pixel = tone_mapper_->apply(input.read(dispatch_id()));
            pixel.w = 1.f;
            output.write(dispatch_id(), pixel);
        };
        shader_ = device().compile(kernel, "tone mapping");
    }

    [[nodiscard]] Command *dispatch() noexcept override {
        return shader_(res<Buffer<float4>>("input"),
                       res<Buffer<float4>>("output"))
            .dispatch(pipeline()->resolution());
    }

    [[nodiscard]] ChannelList inputs() const noexcept override {
        return {
            {"input", "input", false, ResourceFormat::FLOAT4}};
    }

    [[nodiscard]] ChannelList outputs() const noexcept override {
        return {
            {"output", "output", false, ResourceFormat::FLOAT4}};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ToneMappingPass)