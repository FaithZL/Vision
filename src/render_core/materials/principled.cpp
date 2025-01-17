//
// Created by Zero on 2025/1/16.
//

#include <utility>

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class PrincipledBxDFSet : public BxDFSet {
private:
    const SampledWavelengths *swl_{};
    DCSP<Fresnel> fresnel_{};
    optional<LambertReflection> diffuse_;
};

class PrincipledMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(metallic)
    VS_MAKE_SLOT(ior)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(spec_tint)
    VS_MAKE_SLOT(anisotropic)

    VS_MAKE_SLOT(sheen_weight)
    VS_MAKE_SLOT(sheen_roughness)
    VS_MAKE_SLOT(sheen_tint)

    VS_MAKE_SLOT(clearcoat_weight)
    VS_MAKE_SLOT(clearcoat_roughness)
    VS_MAKE_SLOT(clearcoat_tint)

    VS_MAKE_SLOT(subsurface_weight)
    VS_MAKE_SLOT(subsurface_radius)
    VS_MAKE_SLOT(subsurface_scale)

    VS_MAKE_SLOT(spec_trans)
    VS_MAKE_SLOT(flatness)
    VS_MAKE_SLOT(diff_trans)

public:
    PrincipledMaterial() = default;
    explicit PrincipledMaterial(const MaterialDesc &desc)
        : Material(desc) {

#define INIT_SLOT(name, default_value, type) \
    name##_.set(Slot::create_slot(desc.slot(#name, default_value, type)));

        INIT_SLOT(color, make_float3(1.f), Albedo);
        INIT_SLOT(metallic, 0.f, Number);
        INIT_SLOT(ior, 1.5f, Number);
        INIT_SLOT(roughness, 0.5f, Number);
        INIT_SLOT(spec_tint, make_float3(0.f), Albedo);
        INIT_SLOT(anisotropic, 0.f, Number);

        INIT_SLOT(sheen_weight, 0.f, Number);
        INIT_SLOT(sheen_roughness, 0.5f, Number);
        INIT_SLOT(sheen_tint, make_float3(1.f), Albedo);

        INIT_SLOT(clearcoat_weight, 0.3f, Number);
        INIT_SLOT(clearcoat_roughness, 0.2f, Number);
        INIT_SLOT(clearcoat_tint, make_float3(1.f), Albedo);

        INIT_SLOT(subsurface_weight, 0.3f, Number);
        INIT_SLOT(subsurface_radius, make_float3(1.f), Number);
        INIT_SLOT(subsurface_scale, 0.2f, Number);

        INIT_SLOT(spec_trans, 0.f, Number);
        INIT_SLOT(flatness, 0.f, Number);
        INIT_SLOT(diff_trans, 0.f, Number);

#undef INIT_SLOT
        init_slot_cursor(&color_, &diff_trans_);
    }
    void restore(RuntimeObject *old_obj) noexcept override {
        Material::restore(old_obj);
    }

};

}