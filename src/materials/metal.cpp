//
// Created by Zero on 28/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class FresnelConductor : public Fresnel {
private:
    Float3 _eta, _k;

public:
    FresnelConductor(Float3 eta, Float3 k)
        : _eta(eta), _k(k) {}
    [[nodiscard]] Float3 evaluate(Float abs_cos_theta) const noexcept override {
        return fresnel_complex<D>(abs_cos_theta, _eta, _k);
    }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelConductor>(_eta, _k);
    }
};

class MetalBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _refl;

public:
    MetalBSDF(const Interaction &si,
              const SP<Fresnel> &fresnel,
              MicrofacetReflection refl)
        : BSDF(si), _fresnel(fresnel), _refl(move(refl)) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return _refl.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        return _refl.safe_evaluate(wo, wi, _fresnel->clone());
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uchar flag, Sampler *sampler) const noexcept override {
        return _refl.sample(wo, sampler, _fresnel->clone());
    }
};

class MetalMaterial : public Material {
private:
    const Texture *_eta{};
    const Texture *_k{};
    const Texture *_roughness{};
    bool _remapping_roughness{false};

public:
    explicit MetalMaterial(const MaterialDesc &desc)
        : Material(desc),
          _eta(desc.scene->load<Texture>(desc.eta)),
          _k(desc.scene->load<Texture>(desc.k)),
          _roughness(desc.scene->load<Texture>(desc.roughness)),
          _remapping_roughness(desc.remapping_roughness) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si) const noexcept override {
        Float3 kr = make_float3(1.f);
        Float2 alpha = Texture::eval(_roughness, si, 0.0001f).xy();
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        Float3 eta = Texture::eval(_eta, si, 1.5f).xyz();
        Float3 k = Texture::eval(_k, si, 0.f).xyz();
        auto microfacet = make_shared<Microfacet<D>>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelConductor>(eta, k);
        MicrofacetReflection bxdf(kr, microfacet);
        return make_unique<MetalBSDF>(si, fresnel, move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MetalMaterial)