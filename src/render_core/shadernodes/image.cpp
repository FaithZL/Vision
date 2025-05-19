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
    VS_MAKE_SLOT(vector);
    RegistrableTexture *texture_{};
    EncodedData<uint> tex_id_{};
    ShaderNodeDesc desc_;
    mutable optional<float_array> cache_;

public:
    ImageNode() = default;
    explicit ImageNode(const ShaderNodeDesc &desc)
        : ShaderNode(desc),
          desc_(desc),
          texture_(&Global::instance().pipeline()->image_pool().obtain_texture(desc)) {
        tex_id_ = texture_->index();
    }
    OC_ENCODABLE_FUNC(ShaderNode, vector_, tex_id_)
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(ShaderNode, vector_, texture_, tex_id_, desc_)

    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        vector_.set(graph().construct_slot(desc, "vector", Number));
    }

    void reload(ocarina::Widgets *widgets) noexcept {
        fs::path path = texture_->host_tex().path();
        if (Widgets::open_file_dialog(path)) {
            desc_.set_value("fn", path.string());
            desc_.reset_hash();
            texture_ = &Global::instance().pipeline()->image_pool().obtain_texture(desc_);
            texture_->upload_immediately();
            tex_id_.hv() = texture_->index().hv();
            changed_ = true;
        }
    }

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->button_click("reload", [&] {
            reload(widgets);
        });
        widgets->image(texture_->host_tex());
    }

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->text(name_.c_str());
        widgets->same_line();
        widgets->use_tree("open", [&] {
            render_sub_UI(widgets);
        });
        return true;
    }

    [[nodiscard]] float_array evaluate(const string &key, const AttrEvalContext &ctx,
                                       const SampledWavelengths &swl) const noexcept override {
        float_array value = evaluate(ctx, swl);
        if (key == "Alpha") {
            if (channel_num() < 4) {
                return float_array::create(0);
            }
            return value.w();
        } else if (key == "Color") {
            return value.xyz();
        }
        return value;
    }

    [[nodiscard]] uint channel_num() const noexcept { return texture_->host_tex().channel_num(); }

    [[nodiscard]] float_array evaluate(const AttrEvalContext &ctx,
                                       const SampledWavelengths &swl) const noexcept override {
        if (!cache_) {
            AttrEvalContext ctx_processed = vector_.evaluate(ctx, swl);
            float_array value = pipeline()->tex_var(*tex_id_).sample(channel_num(), ctx_processed.uv);
            cache_.emplace(value);
        }
        return *cache_;
    }

    void on_after_decode() const noexcept override {
        cache_.reset();
    }

    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return texture_->host_tex().average_vector();
    }
    [[nodiscard]] uint2 resolution() const noexcept override {
        return texture_->device_tex()->resolution().xy();
    }
    void for_each_pixel(const function<Image::foreach_signature> &func) const noexcept override {
        texture_->host_tex().for_each_pixel(func);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, ImageNode)
//VS_REGISTER_CURRENT_PATH(0, "vision-shadernode-image.dll")