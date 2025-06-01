//
// Created by ling.zhu on 2025/5/22.
//

#include "base/shader_graph/shader_graph.h"
#include "GUI/widgets.h"

namespace vision {

class FresnelNode : public ShaderNode {
private:
    VS_MAKE_SLOT(ior);
    VS_MAKE_SLOT(normal);

public:
    FresnelNode() = default;
    explicit FresnelNode(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC_(fresnel)
    VS_MAKE_GUI_STATUS_FUNC(ShaderNode, ior_, normal_)
    OC_ENCODABLE_FUNC(ShaderNode, ior_, normal_)
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        bool ret = widgets->use_tree("detail", [&] {
            ior_.render_UI(widgets);
            normal_.render_UI(widgets);
        });
        return ret;
    }

    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        VS_INIT_SLOT(ior, 1.5f, Number).set_range(0, 20);
        VS_INIT_SLOT(normal, make_float3(0, 0, 0), Number).set_range(0, 1);
    }

    [[nodiscard]] AttrEvalContext evaluate(const std::string &key, const vision::AttrEvalContext &ctx,
                                           const vision::SampledWavelengths &swl) const noexcept override {

        return ctx;
    }
};

class VectorMapping : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(vector_mapping)

private:
    std::string type_;
    VS_MAKE_SLOT(vector);
    VS_MAKE_SLOT(location);
    VS_MAKE_SLOT(rotation);
    VS_MAKE_SLOT(scale);

public:
    VectorMapping() = default;
    explicit VectorMapping(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}

    VS_MAKE_GUI_STATUS_FUNC(ShaderNode, vector_, location_, rotation_, scale_)
    OC_ENCODABLE_FUNC(ShaderNode, vector_, location_, rotation_, scale_)

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        bool ret = widgets->use_tree("detail", [&] {
            vector_.render_UI(widgets);
            location_.render_UI(widgets);
            rotation_.render_UI(widgets);
            scale_.render_UI(widgets);
        });
        return ret;
    }

    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        VS_INIT_SLOT(vector, make_float3(0, 0, 0), Number);
        VS_INIT_SLOT(location, make_float3(0, 0, 0), Number).set_range(0, 100);
        VS_INIT_SLOT(rotation, make_float3(0, 0, 0), Number).set_range(0, 360);
        VS_INIT_SLOT(scale, make_float3(1), Number).set_range(0.001, 100);
    }

    [[nodiscard]] AttrEvalContext evaluate(const std::string &key, const vision::AttrEvalContext &ctx,
                                           const vision::SampledWavelengths &swl) const noexcept override {
        Float3 uvw = vector_.evaluate(ctx, swl).uvw();
        Float3 s = scale_.evaluate(ctx, swl)->as_vec3();
        Float3 angle = rotation_.evaluate(ctx, swl)->as_vec3();
        Float3 pos = location_.evaluate(ctx, swl)->as_vec3();
        Float4x4 rotation = rotation_y(angle.y) * rotation_z(angle.z) * rotation_x(angle.x);
        Float4x4 trs = translation(pos) * rotation * scale(s);
        uvw = transform_point(trs, uvw);
        AttrEvalContext ctx_processed{float_array::from_vec(uvw)};
        return ctx_processed;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, FresnelNode, fresnel)
VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, VectorMapping, vector_mapping)
VS_REGISTER_CURRENT_FILE