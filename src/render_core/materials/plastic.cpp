//
// Created by ling.zhu on 2025/2/22.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "math/warp.h"

namespace vision {

class PlasticLobe : public MicrofacetLobe {
private:
    DCSP<BxDF> diffuse_;

protected:
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        return hash64(MicrofacetLobe::compute_topology_hash(),
                      diffuse_->topology_hash());
    }

public:
    PlasticLobe(const SP<Fresnel> &fresnel, UP<MicrofacetBxDF> refl, UP<BxDF> diff)
        : MicrofacetLobe(fresnel, std::move(refl)),
          diffuse_(std::move(diff)) {}
    
};

class PlasticMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(ior)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(anisotropic)
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MicrofacetLobe)
public:
    PlasticMaterial() = default;
    explicit PlasticMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {}
    VS_HOTFIX_MAKE_RESTORE(Material, remapping_roughness_, alpha_threshold_)
    void initialize_(const vision::NodeDesc &node_desc) noexcept override {
        VS_CAST_DESC
        Material::initialize_(node_desc);
        INIT_SLOT(color, make_float3(1.f), Albedo);
        INIT_SLOT(ior, 1.5f, Number).set_range(1.003, 5);
        INIT_SLOT(roughness, 0.5f, Number).set_range(0.0001f, 1.f);
        INIT_SLOT(anisotropic, 0.f, Number).set_range(-1, 1);
        init_slot_cursor(&color_, &anisotropic_);
    }

    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum Rd = color_.eval_albedo_spectrum(it, swl).sample;
        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl).as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);

        alpha = remapping_roughness_ ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        return nullptr;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, PlasticMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-plastic.dll")