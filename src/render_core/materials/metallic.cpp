//
// Created by ling.zhu on 2025/3/15.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "math/warp.h"

namespace vision {
class MetallicMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(edge_tint)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(anisotropic)
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MicrofacetLobe)
public:
    MetallicMaterial() = default;
    explicit MetallicMaterial(const MaterialDesc &desc)
        : Material(desc) {}

    void initialize_slots(const vision::Material::Desc &desc) noexcept override {
        Material::initialize_slots(desc);
        VS_INIT_SLOT(color, make_float3(1.f), Albedo);
        VS_INIT_SLOT(edge_tint, make_float3(1.f), Albedo);
        VS_INIT_SLOT(roughness, 0.5f, Number).set_range(0.0001f, 1.f);
        VS_INIT_SLOT(anisotropic, 0.f, Number).set_range(-1, 1);
        init_slot_cursor(&color_, &anisotropic_);
    }

    void prepare() noexcept override {
        MetallicLobe::prepare();
    }

    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(Material, remapping_roughness_, alpha_threshold_)
    [[nodiscard]] UP<Lobe> create_lobe_set(const Interaction &it, const SampledWavelengths &swl) const noexcept override {
        auto shading_frame = compute_shading_frame(it, swl);
        SampledSpectrum color = color_.eval_albedo_spectrum(it, swl).sample;
        SampledSpectrum edge_tint = edge_tint_.eval_albedo_spectrum(it, swl).sample;
        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl)->as_scalar(), 0.01f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl)->as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);
        Float alpha_min = min(alpha.x, alpha.y);
        Uint flag = select(alpha_min < alpha_threshold_, SurfaceData::NearSpec, SurfaceData::Glossy);
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);

        SP<FresnelF82Tint> fresnel_f82 = make_shared<FresnelF82Tint>(color, swl);
        fresnel_f82->init_from_F82(edge_tint);

        UP<MicrofacetReflection> metal_refl = make_unique<MicrofacetReflection>(color, swl, microfacet);
        return make_unique<MetallicLobe>(fresnel_f82, std::move(metal_refl));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MetallicMaterial)