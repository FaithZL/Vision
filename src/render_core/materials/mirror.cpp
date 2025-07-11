//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/scattering/precomputed_table.h"

namespace vision {

class MirrorLobe : public PureReflectionLobe {
public:
    using PureReflectionLobe::PureReflectionLobe;
    bool compensate() const noexcept override { return true; }
};

class MirrorMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(anisotropic)
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MicrofacetLobe)

public:
    MirrorMaterial() = default;
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {}

    void initialize_slots(const vision::Material::Desc &desc) noexcept override {
        Material::initialize_slots(desc);
        VS_INIT_SLOT(color, make_float3(1.f), Albedo);
        VS_INIT_SLOT(roughness, 0.001f, Number).set_range(0.0001f, 1.f);
        VS_INIT_SLOT(anisotropic, 0.f, Number).set_range(-1, 1);
        init_slot_cursor(&color_, &anisotropic_);
    }

    void prepare() noexcept override {
        MirrorLobe::prepare();
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(Material, remapping_roughness_, alpha_threshold_)
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->input_float("alpha_threshold", &alpha_threshold_, 0.001, 0.002);
        Material::render_sub_UI(widgets);
    }

    [[nodiscard]] vector<PrecomputedLobeTable> precompute() const noexcept override {
        vector<PrecomputedLobeTable> ret;
        ret.push_back(precompute_lobe<PureReflectionLobe>(make_uint3(uint2(PureReflectionLobe::lut_res), 1u)));
        return ret;
    }

protected:
    [[nodiscard]] UP<Lobe> create_lobe_set(const Interaction &it, const SampledWavelengths &swl) const noexcept override {
        auto shading_frame = compute_shading_frame(it, swl);
        SampledSpectrum kr = color_.eval_albedo_spectrum(it, swl).sample;
        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl)->as_scalar(), 0.0001f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl)->as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);
        Float alpha_min = min(alpha.x, alpha.y);
        Uint flag = select(alpha_min < alpha_threshold_, SurfaceData::NearSpec, SurfaceData::Glossy);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        SP<Fresnel> fresnel = make_shared<FresnelConstant>(swl);
        UP<MicrofacetReflection> refl = make_unique<MicrofacetReflection>(kr, swl, microfacet);
        return make_unique<MirrorLobe>(fresnel, std::move(refl), flag ,shading_frame);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MirrorMaterial)