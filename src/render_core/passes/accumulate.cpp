//
// Created by Zero on 2023/6/29.
//

#include "render_graph/pass.h"

namespace vision {

class AccumulatePass : public RenderPass {
private:
    Shader<void(Buffer<float4>, Buffer<float4>, uint)> _shader;
public:
    explicit AccumulatePass(const RenderPassDesc &desc)
        : RenderPass(desc) {}

    void compile() noexcept override {
        Kernel kernel = [&](BufferVar<float4> input, BufferVar<float4> output, Uint frame_index) {

        };
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AccumulatePass)