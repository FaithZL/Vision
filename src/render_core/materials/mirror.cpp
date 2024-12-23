//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class MirrorBxDFSet : public BxDFSet {
private:
    DCSP<Fresnel> fresnel_;
    MicrofacetReflection bxdf_;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(fresnel_->type_hash(), bxdf_.type_hash());
    }

public:
    MirrorBxDFSet(const SP<Fresnel> &fresnel, MicrofacetReflection bxdf, const Uint &flag)
        : BxDFSet(flag), fresnel_(fresnel), bxdf_(std::move(bxdf)) {}
    // clang-format off
    VS_MAKE_BxDFSet_ASSIGNMENT(MirrorBxDFSet)
        // clang-format on
        [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return bxdf_.albedo(wo); }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override {
        return bxdf_.safe_evaluate(wo, wi, fresnel_->clone(), mode);
    }

    [[nodiscard]] BSDFSample sample_delta_local(const Float3 &wo, TSampler &sampler) const noexcept override {
        Float3 wi = make_float3(-wo.xy(), wo.z);
        BSDFSample ret{bxdf_.swl()};
        ret.wi = wi;
        ret.eval = bxdf_.evaluate(wo, wi, fresnel_->clone(), All);
        return ret;
    }
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        return bxdf_.sample(wo, sampler, fresnel_->clone());
    }

    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override {
        return bxdf_.sample_wi(wo, sampler->next_2d(), fresnel_->clone());
    }
};

class MirrorMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(anisotropic)
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<MirrorBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    MirrorMaterial() = default;
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {
        color_.set(Slot::create_slot(desc.slot("color", make_float3(1.f), Albedo)));
        roughness_.set(Slot::create_slot(desc.slot("roughness", 0.0001f)));
        anisotropic_.set(Slot::create_slot(desc.slot("anisotropic", 0.f, -1.f, 1.f)));
        init_slot_cursor(&color_, 3);
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

        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelNoOp>(swl, pipeline());
        MicrofacetReflection bxdf(kr, swl, microfacet);
        return make_unique<MirrorBxDFSet>(fresnel, ocarina::move(bxdf), flag);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MirrorMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-mirror.dll")