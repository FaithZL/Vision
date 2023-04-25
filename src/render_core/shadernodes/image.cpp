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
    Serialize<uint> _tex_id{};

public:
    explicit Image(const ShaderNodeDesc &desc)
        : ShaderNode(desc),
          _image_wrapper(desc.scene->render_pipeline()->obtain_image(desc)) {
        _tex_id = _image_wrapper.id();
    }
    OC_SERIALIZABLE_FUNC(float, _tex_id)
    [[nodiscard]] bool is_zero() const noexcept override { return false; }
    void fill_datas(ManagedWrapper<float>&datas) const noexcept override {
        datas.push_back(bit_cast<float>(_image_wrapper.id()));
    }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return render_pipeline()->tex(_image_wrapper.id()).sample(3, ctx.uv);
    }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl,
                                        const DataAccessor<float> *da) const noexcept override {
        Uint index = da->byte_read<uint>();
        return render_pipeline()->tex(index).sample(3, ctx.uv);
    }
    [[nodiscard]] uint2 resolution() const noexcept override {
        return _image_wrapper.texture()->resolution().xy();
    }
    [[nodiscard]] uint datas_size() const noexcept override {
        return sizeof(_image_wrapper.id());
    }
    void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept override {
        _image_wrapper.image().for_each_pixel(func);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Image)