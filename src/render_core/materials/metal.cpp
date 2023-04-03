//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "metal_ior.h"

namespace vision {

class FresnelConductor : public Fresnel {
private:
    SampledSpectrum _eta, _k;

public:
    FresnelConductor(const SampledSpectrum &eta,const SampledSpectrum &k, const SampledWavelengths &swl, const RenderPipeline *rp)
        : Fresnel(swl, rp), _eta(eta), _k(k) {}
    [[nodiscard]] SampledSpectrum evaluate(Float abs_cos_theta) const noexcept override {
        return fresnel_complex(abs_cos_theta, _eta, _k);
    }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelConductor>(_eta, _k, _swl, _rp);
    }
};

class ConductorBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _refl;

public:
    ConductorBSDF(const Interaction &it,
              const SP<Fresnel> &fresnel,
              MicrofacetReflection refl)
        : BSDF(it, refl.swl()), _fresnel(fresnel), _refl(move(refl)) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _refl.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        return _refl.safe_evaluate(wo, wi, _fresnel->clone());
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _refl.sample(wo, sampler, _fresnel->clone());
    }
};

class MetalMaterial : public Material {
private:
    string _material_name{};
    SPD _spd_eta;
    SPD _spd_k;
    Slot _roughness{};
    bool _remapping_roughness{false};

public:
    explicit MetalMaterial(const MaterialDesc &desc)
        : Material(desc),
//          _material_name(desc["material_name"].as_string()),
          _spd_eta(desc.scene->render_pipeline()),
          _spd_k(desc.scene->render_pipeline()),
          _roughness(_scene->create_slot(desc.slot("roughness", make_float2(0.01f)))),
          _remapping_roughness(desc["remapping_roughness"].as_bool(false)) {
        const ComplexIor &complex_ior = ComplexIorTable::instance()->get_ior(_material_name);
        _spd_eta.init(complex_ior.eta);
        _spd_k.init(complex_ior.k);
    }

    void prepare() noexcept override {
        _spd_eta.prepare();
        _spd_k.prepare();
    }

    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_roughness.type_hash(), _material_name);
    }

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr{swl.dimension(), 1.f};
        Float2 alpha = _roughness.evaluate(it).to_vec2();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        SampledSpectrum eta = _spd_eta.eval(swl);
        SampledSpectrum k = _spd_k.eval(swl);
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelConductor>(eta, k, swl, render_pipeline());
        MicrofacetReflection bxdf(kr, swl,microfacet);
        return make_unique<ConductorBSDF>(it, fresnel, move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MetalMaterial)