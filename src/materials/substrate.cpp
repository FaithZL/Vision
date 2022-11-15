//
// Created by Zero on 28/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"
#include "math/warp.h"

namespace vision {

class FresnelBlend : public BxDF {
private:
    Float3 Rd, Rs;
    SP<Microfacet<D>> _microfacet;

public:
    FresnelBlend(Float3 Rd, Float3 Rs, const SP<Microfacet<D>> &m)
        : BxDF(BxDFFlag::Reflection), Rd(Rd), Rs(Rs), _microfacet(m) {}

    [[nodiscard]] Float3 f_diffuse(float3 wo, float3 wi) const noexcept {
        Float3 diffuse = (28.f / (23.f * Pi)) * Rd * (make_float3(1.f) - Rs) *
                         (1 - Pow<5>(1 - .5f * abs_cos_theta(wi))) *
                         (1 - Pow<5>(1 - .5f * abs_cos_theta(wo)));
        return diffuse;
    }
    [[nodiscard]] Float PDF_diffuse(float3 wo, float3 wi) const noexcept {
        return cosine_hemisphere_PDF(abs_cos_theta(wi));
    }
//    [[nodiscard]] Float3 f_specular(float3 wo, float3 wi) const noexcept {
//        Float3 wh = wi + wo;
//        wh = normalize(wh);
//        Float3 specular = _microfacet->D_(wh) /(4 * abs_dot(wi, wh) * max(abs_cos_theta(wi), abs_cos_theta(wo))) *
//            fresnel_schlick(Rs, dot(wi, wh));
//        return specular;
//    }
    [[nodiscard]] Float PDF_specular(float3 wo, float3 wi) const noexcept;
};

class SubstrateMaterial : public Material {
};

}// namespace vision