//
// Created by Zero on 09/09/2022.
//

#include "base/shader_graph/shader_node.h"
#include "rhi/common.h"
#include "base/mgr/pipeline.h"
#include "base/mgr/global.h"

namespace vision {
using namespace ocarina;
class Image : public ShaderNode {
private:
    const ImageWrapper &_image_wrapper;
    Serial<uint> _tex_id{};

public:
    explicit Image(const ShaderNodeDesc &desc)
        : ShaderNode(desc),
          _image_wrapper(Global::instance().pipeline()->obtain_image(desc)) {
        _tex_id = _image_wrapper.id();
    }
    OC_SERIALIZABLE_FUNC(_tex_id)
    [[nodiscard]] bool is_zero() const noexcept override { return false; }

    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return pipeline()->tex(*_tex_id).sample(3, ctx.uv);
    }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return _image_wrapper.image().average_vector();
    }
    [[nodiscard]] uint2 resolution() const noexcept override {
        return _image_wrapper.texture()->resolution().xy();
    }
    void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept override {
        _image_wrapper.image().for_each_pixel(func);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Image)