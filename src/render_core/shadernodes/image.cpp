//
// Created by Zero on 09/09/2022.
//

#include "base/shader_graph/shader_node.h"
#include "rhi/common.h"
#include "base/mgr/render_pipeline.h"

namespace vision {
using namespace ocarina;
class Image : public ShaderNode {
private:
    const ImageWrapper &_image_wrapper;

public:
    explicit Image(const ShaderNodeDesc &desc)
        : ShaderNode(desc),
          _image_wrapper(desc.scene->render_pipeline()->obtain_image(desc)) {}
    [[nodiscard]] bool is_zero() const noexcept override { return false; }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx) const noexcept override {
        return render_pipeline()->tex(_image_wrapper.id()).sample(3, ctx.uv);
    }
    [[nodiscard]] Array<float> _eval(const AttrEvalContext &ctx,
                                     uint type_index,
                                     const Uint &data_offset) const noexcept override {
        OC_ASSERT(false);
        return Array<float>(1u);
    }
    [[nodiscard]] uint2 resolution() const noexcept override {
        return _image_wrapper.texture()->resolution().xy();
    }
    [[nodiscard]] uint data_size() const noexcept override {
        return sizeof(_image_wrapper.id());
    }
    void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept override {
        _image_wrapper.image().for_each_pixel(func);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Image)