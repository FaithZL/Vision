//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "precomputed_table.inl.h"

namespace vision {

class MirrorBxDFSet : public MicrofacetBxDFSet {
private:
    bool compensate_{true};

public:
    using MicrofacetBxDFSet::MicrofacetBxDFSet;

    static constexpr const char *lut_name = "MirrorBxDFSet::lut";
    static constexpr uint lut_res = 32;

    [[nodiscard]] Float compensate_factor(const Float3 &wo) const noexcept {
        Float alpha = bxdf()->alpha_average();
        Float ret = MaterialLut::instance().sample(lut_name, 1, make_float2(alpha, cos_theta(wo))).as_scalar();
        return 1.f / ret;
    }

    static void prepare() {
        MaterialLut::instance().load_lut(lut_name, make_uint2(lut_res),
                                         PixelStorage::FLOAT1,
                                         addressof(MirrorBxDFSet_Table));
    }

    SampledSpectrum albedo(const ocarina::Float &cos_theta) const noexcept override {
        return bxdf()->albedo(cos_theta);
    }

    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override {
        BSDFSample bs = MicrofacetBxDFSet::sample_local(wo, flag, sampler);
        if (compensate_) {
            bs.eval.f *= compensate_factor(wo);
        }
        return bs;
    }

    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override {
        ScatterEval se = MicrofacetBxDFSet::evaluate_local(wo, wi, mode, flag);
        if(compensate_) {
            se.f *= compensate_factor(wo);
        }
        return se;
    }

    /// for precompute begin
    static constexpr const char *name = "MirrorBxDFSet";
    static UP<MirrorBxDFSet> create_for_precompute(const SampledWavelengths &swl) noexcept {
        SP<Fresnel> fresnel = make_shared<FresnelConstant>(swl);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(0.00f, 0.0f, false);
        UP<MicrofacetBxDF> bxdf = make_unique<MicrofacetReflection>(SampledSpectrum::one(swl), swl, microfacet);
        auto ret = make_unique<MirrorBxDFSet>(fresnel, std::move(bxdf));
        ret->compensate_ = false;
        return ret;
    }
    void from_ratio_z(ocarina::Float z) noexcept override {
        // empty
    }
    /// for precompute end
};

class MirrorMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(anisotropic)
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MicrofacetBxDFSet)

public:
    MirrorMaterial() = default;
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {
        color_.set(Slot::create_slot(desc.slot("color", make_float3(1.f), Albedo)));
        roughness_.set(Slot::create_slot(desc.slot("roughness", 0.0001f)));
        anisotropic_.set(Slot::create_slot(desc.slot("anisotropic", 0.f)))->set_range(-1, 1);
        init_slot_cursor(&color_, &anisotropic_);
    }
    void prepare() noexcept override {
        MirrorBxDFSet::prepare();
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(Material, remapping_roughness_, alpha_threshold_)
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->input_float("alpha_threshold", &alpha_threshold_, 0.001, 0.002);
        Material::render_sub_UI(widgets);
    }

    [[nodiscard]] vector<PrecomputedLobeTable> precompute() const noexcept override {
        vector<PrecomputedLobeTable> ret;
        ret.push_back(precompute_lobe<MirrorBxDFSet>(make_uint3(uint2(MirrorBxDFSet::lut_res), 1u)));
        return ret;
    }

protected:
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = color_.eval_albedo_spectrum(it, swl).sample;
        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl).as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);

        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y, MaterialRegistry::instance().sample_visible());
        SP<Fresnel> fresnel = make_shared<FresnelConstant>(swl);
        UP<MicrofacetReflection> refl = make_unique<MicrofacetReflection>(kr, swl, microfacet);
        return make_unique<MirrorBxDFSet>(fresnel, std::move(refl));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MirrorMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-mirror.dll")