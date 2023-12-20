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
    FresnelConductor(const SampledSpectrum &eta, const SampledSpectrum &k, const SampledWavelengths &swl, const Pipeline *rp)
        : Fresnel(swl, rp), _eta(eta), _k(k) {}
    [[nodiscard]] SampledSpectrum evaluate(Float abs_cos_theta) const noexcept override {
        return fresnel_complex(abs_cos_theta, _eta, _k);
    }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelConductor>(_eta, _k, *_swl, _rp);
    }
};

class ConductorBxDFSet : public BxDFSet {
private:
    SP<Fresnel> _fresnel;
    MicrofacetReflection _refl;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_fresnel->type_hash(), _refl.type_hash());
    }

public:
    ConductorBxDFSet(const SP<Fresnel> &fresnel,
                     MicrofacetReflection refl)
        : _fresnel(fresnel), _refl(ocarina::move(refl)) {}
    // clang-format off
    VS_MAKE_BxDFSet_ASSIGNMENT(ConductorBxDFSet)
    ConductorBxDFSet &operator=(const ConductorBxDFSet &other) noexcept {
        *_fresnel = *other._fresnel;
        _refl = other._refl;
        return *this;
    }
    // clang-format on
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _refl.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        return _refl.safe_evaluate(wo, wi, _fresnel->clone());
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _refl.sample(wo, sampler, _fresnel->clone());
    }
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Uint flag,
                                             Sampler *sampler) const noexcept override {
        return _refl.sample_wi(wo, sampler->next_2d(), _fresnel->clone());
    }
};

//    "type" : "metal",
//    "param" : {
//        "material_name" : "Cu",
//        "roughness" : [0.001, 0.001]
//    }
class MetalMaterial : public Material {
private:
    Slot _eta;
    Slot _k;
    Slot _roughness{};
    bool _remapping_roughness{false};

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<ConductorBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    explicit MetalMaterial(const MaterialDesc &desc)
        : Material(desc),
          _roughness(scene().create_slot(desc.slot("roughness", make_float2(0.01f)))),
          _remapping_roughness(desc["remapping_roughness"].as_bool(true)) {
        init_ior(desc);
        init_slot_cursor(&_eta, 3);
    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void init_ior(const MaterialDesc &desc) noexcept {
        const ComplexIor &complex_ior = ComplexIorTable::instance()->get_ior(desc["material_name"].as_string());
        SlotDesc eta_slot;
        SlotDesc k_slot;
        if (spectrum().is_complete()) {
            eta_slot = SlotDesc(ShaderNodeDesc{ShaderNodeType::ESPD, "spd"}, 0);
            eta_slot.node.set_value("value", complex_ior.eta);
            k_slot = SlotDesc(ShaderNodeDesc{ShaderNodeType::ESPD, "spd"}, 0);
            k_slot.node.set_value("value", complex_ior.k);
        } else {
            SPD spd_eta = SPD(complex_ior.eta, nullptr);
            SPD spd_k = SPD(complex_ior.k, nullptr);
            float3 eta = spd_eta.eval(rgb_spectrum_peak_wavelengths);
            float3 k = spd_k.eval(rgb_spectrum_peak_wavelengths);
            eta_slot = desc.slot("", eta);
            k_slot = desc.slot("", k);
        }

        _eta = scene().create_slot(eta_slot);
        _k = scene().create_slot(k_slot);
    }

    void prepare() noexcept override {
        _eta->prepare();
        _k->prepare();
    }

    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr{swl.dimension(), 1.f};
        Float2 alpha = _roughness.evaluate(it, swl).as_vec2();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        SampledSpectrum eta = SampledSpectrum{_eta.evaluate(it, swl)};
        SampledSpectrum k = SampledSpectrum{_k.evaluate(it, swl)};
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelConductor>(eta, k, swl, pipeline());
        MicrofacetReflection bxdf(kr, swl, microfacet);
        return make_unique<ConductorBxDFSet>(fresnel, ocarina::move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MetalMaterial)