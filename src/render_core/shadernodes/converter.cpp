//
// Created by ling.zhu on 2025/5/22.
//

#include "base/shader_graph/shader_graph.h"
#include "GUI/widgets.h"

namespace vision {

#define VS_CALL_RENDER_UI(slot) slot.render_UI(widgets);

#define VS_MAKE_SLOT_RENDER_UI(...)                                             \
    bool render_UI(ocarina::Widgets *widgets) noexcept override {               \
        bool ret = widgets->use_tree(ocarina::format("{} detail", name_), [&] { \
            MAP(VS_CALL_RENDER_UI, ##__VA_ARGS__);                              \
        });                                                                     \
        return ret;                                                             \
    }

#define VS_MAKE_SLOT_FUNC(Super, ...)             \
    VS_MAKE_GUI_STATUS_FUNC(Super, ##__VA_ARGS__) \
    OC_ENCODABLE_FUNC(Super, ##__VA_ARGS__)       \
    VS_MAKE_SLOT_RENDER_UI(##__VA_ARGS__)

class FresnelNode : public ShaderNode {
private:
    VS_MAKE_SLOT(ior);
    VS_MAKE_SLOT(normal);

public:
    FresnelNode() = default;
    explicit FresnelNode(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}

    VS_MAKE_PLUGIN_NAME_FUNC_(fresnel)
    VS_MAKE_SLOT_FUNC(ShaderNode, ior_, normal_)
    VS_HOTFIX_MAKE_RESTORE(ShaderNode, ior_, normal_)

    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        VS_INIT_SLOT(normal, make_float3(0, 0, 0), Number).set_range(0, 1);
        VS_INIT_SLOT(ior, 1.5f, Number).set_range(0, 20);
    }

    [[nodiscard]] AttrEvalContext evaluate(const std::string &key, const vision::AttrEvalContext &ctx,
                                           const vision::SampledWavelengths &swl) const noexcept override {
        Float3 normal = normal_.evaluate(ctx, swl)->as_vec3();
        Float3 wo = ctx.wo();
        Float ior = ior_.evaluate(ctx, swl)->as_scalar();
        Float3 fr = make_float3(fresnel_dielectric(abs_dot(normal, wo), ior));
        return AttrEvalContext{float_array ::from_vec(fr)};
    }
};

class NormalMap : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(normal_map)

private:
    VS_MAKE_SLOT(color);
    VS_MAKE_SLOT(strength);

public:
    NormalMap() = default;
    explicit NormalMap(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}

    VS_MAKE_GUI_STATUS_FUNC(ShaderNode, color_, strength_)
    OC_ENCODABLE_FUNC(ShaderNode, color_, strength_)
    VS_HOTFIX_MAKE_RESTORE(ShaderNode, color_, strength_)

    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        VS_INIT_SLOT(color, make_float3(0.5, 0.5, 1), Albedo);
        VS_INIT_SLOT(strength, 1.f, Number).set_range(-1, 1);
    }

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        bool ret = widgets->use_tree(ocarina::format("{} detail", name_), [&] {
            color_.render_UI(widgets);
            strength_.render_UI(widgets);
        });
        return ret;
    }

    [[nodiscard]] AttrEvalContext evaluate(const AttrEvalContext &ctx,
                                           const SampledWavelengths &swl) const noexcept override {
        Float3 normal = color_.evaluate(ctx, swl)->as_vec3() * 2.f - make_float3(1.f);
        Float s = strength_.evaluate(ctx, swl)->as_scalar();
        normal.x *= s;
        normal.y *= s;
        normal = ocarina::normalize(normal);
        return AttrEvalContext(float_array::from_vec(normal));
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
    VS_HOTFIX_MAKE_RESTORE(ShaderNode, vector_, location_, rotation_, scale_, type_)

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        bool ret = widgets->use_tree(ocarina::format("{} detail", name_), [&] {
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

class Clamp : public ShaderNodeMultiSlot {
public:
    using ShaderNodeMultiSlot::ShaderNodeMultiSlot;
    VS_MAKE_PLUGIN_NAME_FUNC_(clamp)

private:
    VS_MAKE_SLOT(min);
    VS_MAKE_SLOT(max);
    VS_MAKE_SLOT(value);

public:
    Clamp() = default;
    explicit Clamp(const ShaderNodeDesc &desc)
        : ShaderNodeMultiSlot(desc) {}

    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        VS_INIT_SLOT(min, 1.f, Number).set_range(-100, 100);
        VS_INIT_SLOT(max, 1.f, Number).set_range(-100, 100);
        VS_INIT_SLOT(value, 1.f, Number).set_range(-100, 100);
        init_slot_cursor(&min_, &value_);
    }

    [[nodiscard]] AttrEvalContext evaluate(const AttrEvalContext &ctx,
                                           const SampledWavelengths &swl) const noexcept override {
        Float min = min_.evaluate(ctx, swl)->as_scalar();
        Float max = max_.evaluate(ctx, swl)->as_scalar();
        float_array value = value_.evaluate(ctx, swl).array;
        return {value.clamp(min, max)};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, FresnelNode, fresnel)
VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, NormalMap, normal_map)
VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, VectorMapping, vector_mapping)
VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, Clamp, clamp)
VS_REGISTER_CURRENT_FILE