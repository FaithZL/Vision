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
    DCUP<BxDF> diffuse_;

protected:
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        return hash64(MicrofacetLobe::compute_topology_hash(),
                      diffuse_->topology_hash());
    }

public:
    PlasticLobe(const SP<Fresnel> &fresnel, UP<MicrofacetBxDF> refl, UP<BxDF> diff)
        : MicrofacetLobe(fresnel, std::move(refl)),
          diffuse_(std::move(diff)) {}

    [[nodiscard]] Float PDF_diffuse(const Float3 &wo, const Float3 &wi) const noexcept {
        return cosine_hemisphere_PDF(abs_cos_theta(wi));
    }

    [[nodiscard]] Float PDF_specular(const Float3 &wo, const Float3 &wi) const noexcept {
        Float3 wh = normalize(wo + wi);
        Float ret = microfacet()->PDF_wi_reflection(wo, wh);
        return ret;
    }

    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag, TransportMode tm) const noexcept override {
        ScatterEval ret{*swl()};
        Float3 wh = normalize(wo + wi);
        SampledSpectrum F = fresnel_->evaluate(abs_dot(wh, wo));
        if (BxDF::match_F(mode)) {
            ret.f = diffuse_->f(wo, wi, nullptr, tm) * (1 - F);
            ret.f += microfacet()->BRDF(wo, wh, wi, F);
        }
        if (BxDF::match_PDF(mode)) {
            ret.pdfs = lerp(F.average(), PDF_diffuse(wo, wi), PDF_specular(wo, wi));
        }
        return ret;
    }

    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler, TransportMode tm) const noexcept override {
        return Lobe::sample_local(wo, flag, sampler, tm);
    }

    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override {
        Float3 wh = microfacet()->sample_wh(wo, sampler->next_2d());
        Float d = dot(wo, wh);
        auto fresnel = fresnel_.ptr();
        SampledDirection sd;
        SampledSpectrum F = fresnel->evaluate(abs(d));
        Float uc = sampler->next_1d();
        $if(uc < F.average()) {
            sd.wi = reflect(wo, wh);
            sd.valid = same_hemisphere(wo, sd.wi);
        }
        $else {
            sd.wi = square_to_cosine_hemisphere(sampler->next_2d());
        };
        return sd;
    }
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
        DynamicArray<float> iors = ior_.evaluate(it, swl);

        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl).as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);

        alpha = remapping_roughness_ ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto fresnel = make_shared<FresnelDielectric>(SampledSpectrum{iors}, swl);
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        UP<MicrofacetReflection> refl = make_unique<MicrofacetReflection>(spectrum()->one(), swl, microfacet);
        UP<LambertReflection> diffuse = make_unique<LambertReflection>(Rd, swl);
        return make_unique<PlasticLobe>(fresnel, std::move(refl), std::move(diffuse));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, PlasticMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-plastic.dll")