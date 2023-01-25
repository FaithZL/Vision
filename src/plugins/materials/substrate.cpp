//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/texture.h"
#include "base/mgr/scene.h"
#include "math/warp.h"

namespace vision {

class FresnelBlend : public BxDF {
private:
    SampledSpectrum Rd, Rs;
    SP<Microfacet<D>> _microfacet;

public:
    FresnelBlend(SampledSpectrum Rd, SampledSpectrum Rs, const SampledWavelengths &swl, const SP<Microfacet<D>> &m)
        : BxDF(swl, BxDFFlag::Reflection), Rd(Rd), Rs(Rs), _microfacet(m) {}

    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return Rd; }

    [[nodiscard]] SampledSpectrum f_diffuse(Float3 wo, Float3 wi) const noexcept {
        SampledSpectrum diffuse = (28.f / (23.f * Pi)) * Rd * (SampledSpectrum(swl().dimension(), 1.f) - Rs) *
                          (1 - Pow<5>(1 - .5f * abs_cos_theta(wi))) *
                          (1 - Pow<5>(1 - .5f * abs_cos_theta(wo)));
        return diffuse;
    }
    [[nodiscard]] Float PDF_diffuse(Float3 wo, Float3 wi) const noexcept {
        return cosine_hemisphere_PDF(abs_cos_theta(wi));
    }
    [[nodiscard]] SampledSpectrum f_specular(Float3 wo, Float3 wi) const noexcept {
        Float3 wh = wi + wo;
        wh = normalize(wh);
        SampledSpectrum specular = _microfacet->D_(wh) / (4 * abs_dot(wi, wh) * max(abs_cos_theta(wi), abs_cos_theta(wo))) *
                          fresnel_schlick(Rs, dot(wi, wh));
        return select(is_zero(wh), 0.f, 1.f) * specular;
    }
    [[nodiscard]] Float PDF_specular(Float3 wo, Float3 wi) const noexcept {
        Float3 wh = normalize(wo + wi);
        Float ret = _microfacet->PDF_wi_reflection(wo, wh);
        return ret;
    }
    [[nodiscard]] SampledSpectrum f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        SampledSpectrum ret = f_specular(wo, wi) + f_diffuse(wo, wi);
        return ret;
    }
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
        return (1.f - fr) * PDF_diffuse(wo, wi) + fr * PDF_specular(wo, wi);
    }
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override {
        BSDFSample ret;
        Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
        $if(u.x < fr) {
            u.x = remapping(u.x, 0.f, fr);
            Float3 wh = _microfacet->sample_wh(wo, u);
            ret.wi = reflect(wo, wh);
            ret.eval.f = f_specular(wo, ret.wi);
            ret.eval.pdf = PDF_specular(wo, ret.wi);
            ret.eval.pdf = select(safe(wo, ret.wi), ret.eval.pdf, 0.f) * fr;
        }
        $else {
            u.x = remapping(u.x, fr, 1.f);
            ret.wi = square_to_cosine_hemisphere(u);
            ret.wi.z = select(wo.z < 0, -ret.wi.z, ret.wi.z);
            ret.eval.f = f_diffuse(wo, ret.wi);
            ret.eval.pdf = PDF_diffuse(wo, ret.wi) * (1 - fr);
        };
        return ret;
    }

    [[nodiscard]] BSDFSample sample(Float3 wo, Sampler *sampler,
                                        SP<Fresnel> fresnel) const noexcept override {
        BSDFSample ret;
        Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
        Float2 u = sampler->next_2d();
        $if(u.x < fr) {
            u.x = remapping(u.x, 0.f, fr);
            Float3 wh = _microfacet->sample_wh(wo, u);
            ret.wi = reflect(wo, wh);
            ret.eval.f = f_specular(wo, ret.wi);
            ret.eval.pdf = PDF_specular(wo, ret.wi);
            ret.eval.pdf = select(safe(wo, ret.wi), ret.eval.pdf, 0.f) * fr;
        }
        $else {
            u.x = remapping(u.x, fr, 1.f);
            ret.wi = square_to_cosine_hemisphere(u);
            ret.wi.z = select(wo.z < 0, -ret.wi.z, ret.wi.z);
            ret.eval.f = f_diffuse(wo, ret.wi);
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
    SubstrateBSDF(const Interaction &si, const SP<Fresnel> &fresnel, FresnelBlend bxdf)
        : BSDF(si), _fresnel(fresnel), _bxdf(std::move(bxdf)) {}

    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi, _fresnel->clone());
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uchar flag, Sampler *sampler) const noexcept override {
        return _bxdf.sample(wo, sampler, _fresnel->clone());
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
        : Material(desc), _diff(desc.scene->load<Texture>(desc.color)),
          _spec(desc.scene->load<Texture>(desc.spec)),
          _roughness(desc.scene->load<Texture>(desc.roughness)),
          _remapping_roughness(desc.remapping_roughness) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum Rd = Texture::eval_albedo_spectrum(_diff, si, swl);
        SampledSpectrum Rs = Texture::eval_albedo_spectrum(_spec, si, swl);
        Float2 alpha = Texture::eval(_roughness, si, 0.001f).xy();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelDielectric>(1.5f, swl, render_pipeline());
        FresnelBlend bxdf(Rd, Rs, swl, microfacet);
        return make_unique<SubstrateBSDF>(si, fresnel, move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SubstrateMaterial)