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
class ImageNode : public ShaderNode {
private:
    RegistrableTexture *_texture;
    Serial<uint> _tex_id{};
    ShaderNodeDesc _desc;

public:
    explicit ImageNode(const ShaderNodeDesc &desc)
        : ShaderNode(desc),
          _desc(desc),
          _texture(&Global::instance().pipeline()->image_pool().obtain_texture(desc)) {
        _tex_id = _texture->index();
    }
    OC_SERIALIZABLE_FUNC(ShaderNode, _tex_id)
    VS_MAKE_PLUGIN_NAME_FUNC

    void reload(ocarina::Widgets *widgets) noexcept {
        fs::path path = _texture->host_tex().path();
        if (Widgets::open_file_dialog(path)) {
            _desc.set_value("fn", path.string());
            _desc.reset_hash();
            _texture = &Global::instance().pipeline()->image_pool().obtain_texture(_desc);
            _texture->upload_immediately();
            _tex_id.hv() = _texture->index().hv();
            _changed = true;
        }
    }

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->text(_name.c_str());
        widgets->same_line();
        widgets->button_click("reload", [&] {
            reload(widgets);
        });
        widgets->image(_texture->host_tex());
        return true;
    }

    [[nodiscard]] bool is_zero() const noexcept override { return false; }

    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return pipeline()->tex_var(*_tex_id).sample(_texture->host_tex().channel_num(), ctx.uv);
    }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return _texture->host_tex().average_vector();
    }
    [[nodiscard]] uint2 resolution() const noexcept override {
        return _texture->device_tex()->resolution().xy();
    }
    void for_each_pixel(const function<Image::foreach_signature> &func) const noexcept override {
        _texture->host_tex().for_each_pixel(func);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ImageNode)