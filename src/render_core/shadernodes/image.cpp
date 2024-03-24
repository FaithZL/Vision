//
// Created by Zero on 09/09/2022.
//

#include "base/shader_graph/shader_node.h"
#include "rhi/common.h"
#include "base/mgr/pipeline.h"
#include "base/mgr/global.h"
#include "GUI/window.h"

namespace vision {
using namespace ocarina;
class Image : public ShaderNode {
private:
    const RegistrableTexture &_texture;
    Serial<uint> _tex_id{};

public:
    explicit Image(const ShaderNodeDesc &desc)
        : ShaderNode(desc),
          _texture(Global::instance().pipeline()->image_pool().obtain_texture(desc)) {
        _tex_id = _texture.index();
    }
    OC_SERIALIZABLE_FUNC(ShaderNode, _tex_id)
    VS_MAKE_PLUGIN_NAME_FUNC

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->window()->interop(&_texture);
        return true;
    }

    [[nodiscard]] bool is_zero() const noexcept override { return false; }

    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return pipeline()->tex_var(*_tex_id).sample(_texture.host_tex().channel_num(), ctx.uv);
    }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return _texture.host_tex().average_vector();
    }
    [[nodiscard]] uint2 resolution() const noexcept override {
        return _texture.device_tex()->resolution().xy();
    }
    void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept override {
        _texture.host_tex().for_each_pixel(func);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Image)