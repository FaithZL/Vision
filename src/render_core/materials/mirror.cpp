//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class MirrorMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(anisotropic)
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(UniversalReflectBxDFSet)

public:
    MirrorMaterial() = default;
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {
        color_.set(Slot::create_slot(desc.slot("color", make_float3(1.f), Albedo)));
        roughness_.set(Slot::create_slot(desc.slot("roughness", 0.0001f)));
        anisotropic_.set(Slot::create_slot(desc.slot("anisotropic", 0.f, -1.f, 1.f)));
        init_slot_cursor(&color_, &anisotropic_);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(Material, anisotropic_, remapping_roughness_, alpha_threshold_)
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->input_float("alpha_threshold", &alpha_threshold_, 0.001, 0.002);
        Material::render_sub_UI(widgets);
    }

protected:
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = color_.eval_albedo_spectrum(it, swl).sample;
        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl).as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);

        Float alpha_min = min(alpha.x, alpha.y);
        Uint flag = select(alpha_min < alpha_threshold_, SurfaceData::NearSpec, SurfaceData::Glossy);

        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        SP<Fresnel> fresnel = make_shared<FresnelNoOp>(swl);
        UP<BxDF> refl = make_unique<MicrofacetReflection>(kr, swl, microfacet);
        return make_unique<UniversalReflectBxDFSet>(fresnel, std::move(refl));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MirrorMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-mirror.dll")