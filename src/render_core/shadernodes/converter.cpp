//
// Created by ling.zhu on 2025/5/22.
//

#include "base/shader_graph/shader_graph.h"
#include "GUI/widgets.h"

namespace vision {

class FresnelNode : public SlotsShaderNode {
private:
    VS_MAKE_SLOT(ior);
    VS_MAKE_SLOT(normal);

public:
    FresnelNode() = default;
    explicit FresnelNode(const ShaderNodeDesc &desc)
        : SlotsShaderNode(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC_(fresnel)
    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        VS_INIT_SLOT(normal, make_float3(0, 0, 0), Number).set_range(0, 1);
        VS_INIT_SLOT(ior, 1.5f, Number).set_range(0, 20);
        init_slot_cursor(&ior_, &normal_);
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

class NormalMap : public SlotsShaderNode {
public:
    using SlotsShaderNode::SlotsShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(normal_map)

private:
    VS_MAKE_SLOT(color);
    VS_MAKE_SLOT(strength);

public:
    NormalMap() = default;
    explicit NormalMap(const ShaderNodeDesc &desc)
        : SlotsShaderNode(desc) {}

    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        VS_INIT_SLOT(color, make_float3(0.5, 0.5, 1), Albedo);
        VS_INIT_SLOT(strength, 1.f, Number).set_range(-1, 1);
        init_slot_cursor(&color_, &strength_);
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

class VectorMapping : public SlotsShaderNode {
public:
    using SlotsShaderNode::SlotsShaderNode;
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
        : SlotsShaderNode(desc) {}

    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        VS_INIT_SLOT(vector, make_float3(0, 0, 0), Number);
        VS_INIT_SLOT(location, make_float3(0, 0, 0), Number).set_range(0, 100);
        VS_INIT_SLOT(rotation, make_float3(0, 0, 0), Number).set_range(0, 360);
        VS_INIT_SLOT(scale, make_float3(1), Number).set_range(0.001, 100);
        init_slot_cursor(&vector_, &scale_);
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

class Clamp : public SlotsShaderNode {
public:
    using SlotsShaderNode::SlotsShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(clamp)

private:
    VS_MAKE_SLOT(min);
    VS_MAKE_SLOT(max);
    VS_MAKE_SLOT(value);

public:
    Clamp() = default;
    explicit Clamp(const ShaderNodeDesc &desc)
        : SlotsShaderNode(desc) {}

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