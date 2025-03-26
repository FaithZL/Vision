//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "math/warp.h"

namespace vision {

class FresnelBlend : public MicrofacetBxDF {
private:
    SampledSpectrum Rd_, Rs_;

public:
    FresnelBlend(SampledSpectrum Rd, SampledSpectrum Rs, const SampledWavelengths &swl, const SP<Microfacet<D>> &m)
        : MicrofacetBxDF(m, BxDFFlag::Reflection, swl), Rd_(std::move(Rd)), Rs_(std::move(Rs)) {}
    // clang-format off
    VS_MAKE_BxDF_ASSIGNMENT(FresnelBlend)
        // clang-format on
        [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override { return Rd_; }

    [[nodiscard]] SampledSpectrum f_diffuse(const Float3 &wo, const Float3 &wi) const noexcept {
        SampledSpectrum diffuse = (28.f / (23.f * Pi)) * Rd_ * (SampledSpectrum(swl().dimension(), 1.f) - Rs_) *
                                  (1 - Pow<5>(1 - .5f * abs_cos_theta(wi))) *
                                  (1 - Pow<5>(1 - .5f * abs_cos_theta(wo)));
        return diffuse;
    }
    [[nodiscard]] Float PDF_diffuse(const Float3 &wo, const Float3 &wi) const noexcept {
        return cosine_hemisphere_PDF(abs_cos_theta(wi));
    }
    [[nodiscard]] SampledSpectrum f_specular(const Float3 &wo, const Float3 &wi) const noexcept {
        Float3 wh = wi + wo;
        wh = normalize(wh);
        SampledSpectrum specular = microfacet_->bsdf_D(wh) / (4 * abs_dot(wi, wh) * max(abs_cos_theta(wi), abs_cos_theta(wo))) *
                                   fresnel_schlick(Rs_, dot(wi, wh));
        return select(is_zero(wh), 0.f, 1.f) * specular;
    }
    [[nodiscard]] Float PDF_specular(const Float3 &wo, const Float3 &wi) const noexcept {
        Float3 wh = normalize(wo + wi);
        Float ret = microfacet_->PDF_wi_reflection(wo, wh);
        return ret;
    }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel, TransportMode tm) const noexcept override {
        SampledSpectrum ret = f_specular(wo, wi) + f_diffuse(wo, wi);
        return ret;
    }
    [[nodiscard]] Float PDF(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept override {
        Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
        return lerp(fr, PDF_diffuse(wo, wi), PDF_specular(wo, wi));
    }

    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, Float2 u, SP<Fresnel> fresnel) const noexcept override {
        SampledDirection ret;
        Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
        $if(u.x < fr) {
            u.x = remapping(u.x, 0.f, fr);
            Float3 wh = microfacet_->sample_wh(wo, u);
            ret.wi = reflect(wo, wh);
        }
        $else {
            u.x = remapping(u.x, fr, 1.f);
            ret.wi = square_to_cosine_hemisphere(u);
            ret.wi.z = select(wo.z < 0, -ret.wi.z, ret.wi.z);
        };
        ret.valid = true;
        return ret;
    }
};

class SubstrateLobe : public MicrofacetLobe {
public:
    using MicrofacetLobe::MicrofacetLobe;
};

//    "type" : "substrate",
//    "param" : {
//        "roughness" : 0.001,
//        "spec" : [
//            0.04,
//            0.04,
//            0.04
//        ],
//        "color" : [
//            0.725,
//            0.71,
//            0.68
//        ]
//    }
class SubstrateMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(spec)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(anisotropic)
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MicrofacetLobe)

public:
    SubstrateMaterial() = default;
    explicit SubstrateMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {
        INIT_SLOT(color, make_float3(1.f), Albedo);
        INIT_SLOT(spec, make_float3(0.05f), Albedo);
        INIT_SLOT(roughness, 0.5f, Number)->set_range(0.0001f, 1.f);
        INIT_SLOT(anisotropic, 0.f, Number)->set_range(-1, 1);
        init_slot_cursor(&color_, &anisotropic_);
    }
    [[nodiscard]] bool enable_delta() const noexcept override { return false; }
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum Rd = color_.eval_albedo_spectrum(it, swl).sample;
        SampledSpectrum Rs = spec_.eval_albedo_spectrum(it, swl).sample;

        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl).as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);

        alpha = remapping_roughness_ ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);

        auto fresnel = make_shared<FresnelDielectric>(SampledSpectrum{swl.dimension(), 1.5f},
                                                      swl);
        UP<FresnelBlend> refl = make_unique<FresnelBlend>(Rd, Rs, swl, microfacet);
        return make_unique<SubstrateLobe>(fresnel, std::move(refl));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, SubstrateMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-substrate.dll")