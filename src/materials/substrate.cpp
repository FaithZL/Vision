//
// Created by Zero on 28/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"
#include "math/warp.h"
#include "math/sampling.h"

namespace vision {

class FresnelBlend : public BxDF {
private:
    Float3 Rd, Rs;
    SP<Microfacet<D>> _microfacet;

public:
    FresnelBlend(Float3 Rd, Float3 Rs, const SP<Microfacet<D>> &m)
        : BxDF(BxDFFlag::Reflection), Rd(Rd), Rs(Rs), _microfacet(m) {}

    [[nodiscard]] Float3 albedo() const noexcept override { return Rd; }

    [[nodiscard]] Float3 f_diffuse(Float3 wo, Float3 wi) const noexcept {
        Float3 diffuse = (28.f / (23.f * Pi)) * Rd * (make_float3(1.f) - Rs) *
                         (1 - Pow<5>(1 - .5f * abs_cos_theta(wi))) *
                         (1 - Pow<5>(1 - .5f * abs_cos_theta(wo)));
        return diffuse;
    }
    [[nodiscard]] Float PDF_diffuse(Float3 wo, Float3 wi) const noexcept {
        return cosine_hemisphere_PDF(abs_cos_theta(wi));
    }
    [[nodiscard]] Float3 f_specular(Float3 wo, Float3 wi) const noexcept {
        Float3 wh = wi + wo;
        wh = normalize(wh);
        Float3 specular = _microfacet->D_(wh) / (4 * abs_dot(wi, wh) * max(abs_cos_theta(wi), abs_cos_theta(wo))) *
                          fresnel_schlick(Rs, dot(wi, wh));
        return select(is_zero(wh), make_float3(0.f), specular);
    }
    [[nodiscard]] Float PDF_specular(Float3 wo, Float3 wi) const noexcept {
        Float3 wh = normalize(wo + wi);
        Float ret = _microfacet->PDF_wi_reflection(wo, wh);
        return ret;
    }

    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        Float3 ret = f_specular(wo, wi) + f_diffuse(wo, wi);
        return ret * abs_cos_theta(wi);
    }
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        return 0.5f * (PDF_diffuse(wo, wi) + PDF_specular(wo, wi));
    }
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override {
        BSDFSample ret;
        Float fr = 0.5f;
        $if(u.x < fr) {
            u.x = remapping(u.x, 0.f, fr);
            Float3 wh = _microfacet->sample_wh(wo, u);
            ret.wi = reflect(wo, wh);
            ret.eval.f = f_specular(wo, ret.wi) * abs_cos_theta(ret.wi);
            ret.eval.pdf = PDF_specular(wo, ret.wi);
            ret.eval.pdf = select(safe(wo, ret.wi), ret.eval.pdf, 0.f) * fr;
        }
        $else {
            u.x = remapping(u.x, fr, 1.f);
            ret.wi = square_to_cosine_hemisphere(u);
            ret.wi.z = select(wo.z < 0, -ret.wi.z, ret.wi.z);
            ret.eval.f = f_diffuse(wo, ret.wi) * abs_cos_theta(ret.wi);
            ret.eval.pdf = PDF_diffuse(wo, ret.wi) * (1 - fr);
        };
        return ret;
    }
};

class SubstrateBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel;
    FresnelBlend _bxdf;

public:
    SubstrateBSDF(const SurfaceInteraction &si, const SP<Fresnel> &fresnel, FresnelBlend bxdf)
        : BSDF(si), _fresnel(fresnel), _bxdf(std::move(bxdf)) {}

    [[nodiscard]] Float3 albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] BSDFEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi, _fresnel->clone());
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Float uc, Float2 u,
                                          Uchar flag) const noexcept override {
        return _bxdf.sample(wo, u, _fresnel->clone());
    }
};

class SubstrateMaterial : public Material {
private:
    const Texture *_diff{};
    const Texture *_spec{};
    const Texture *_roughness{};
    bool _remapping_roughness{false};

public:
    explicit SubstrateMaterial(const MaterialDesc &desc)
        : Material(desc), _diff(desc.scene->load<Texture>(desc.Rd)),
          _spec(desc.scene->load<Texture>(desc.Rs)),
          _roughness(desc.scene->load<Texture>(desc.roughness)),
          _remapping_roughness(desc.remapping_roughness) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept override {
        Float3 Rd = _diff ? _diff->eval(si).xyz() : make_float3(0.f);
        Float3 Rs = _spec ? _spec->eval(si).xyz() : make_float3(0.f);
        Float2 alpha = _roughness ? _roughness->eval(si).xy() : make_float2(0.001f);
        auto microfacet = make_shared<Microfacet<D>>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelDielectric>(1.5f);
        FresnelBlend bxdf(Rd, Rs, microfacet);
        return make_unique<SubstrateBSDF>(si, fresnel, move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SubstrateMaterial)