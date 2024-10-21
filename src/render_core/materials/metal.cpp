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
    SampledSpectrum eta_, k_;

public:
    FresnelConductor(const SampledSpectrum &eta, const SampledSpectrum &k,
                     const SampledWavelengths &swl, const Pipeline *rp)
        : Fresnel(swl, rp), eta_(eta), k_(k) {}
    [[nodiscard]] SampledSpectrum evaluate(Float abs_cos_theta) const noexcept override {
        return fresnel_complex(abs_cos_theta, eta_, k_);
    }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelConductor>(eta_, k_, *swl_, rp_);
    }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelConductor)
};

class ConductorBxDFSet : public BxDFSet {
private:
    DCSP<Fresnel> fresnel_;
    MicrofacetReflection refl_;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(fresnel_->type_hash(), refl_.type_hash());
    }

public:
    ConductorBxDFSet(const SP<Fresnel> &fresnel,
                     MicrofacetReflection refl,
                     const Uint &flag)
        : BxDFSet(flag), fresnel_(fresnel), refl_(ocarina::move(refl)) {}
    VS_MAKE_BxDFSet_ASSIGNMENT(ConductorBxDFSet)
        [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return refl_.albedo(wo); }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3& wi, MaterialEvalMode mode, const Uint &flag) const noexcept override {
        return refl_.safe_evaluate(wo, wi, fresnel_->clone(), mode);
    }
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        return refl_.sample(wo, sampler, fresnel_->clone());
    }
    [[nodiscard]] BSDFSample sample_delta_local(const Float3 &wo, TSampler &sampler) const noexcept override {
        Float3 wi = make_float3(-wo.xy(), wo.z);
        BSDFSample ret{refl_.swl()};
        ret.wi = wi;
        ret.eval = refl_.evaluate(wo, wi, fresnel_->clone(), All);
        return ret;
    }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override {
        return refl_.sample_wi(wo, sampler->next_2d(), fresnel_->clone());
    }
};

//    "type" : "metal",
//    "param" : {
//        "material_name" : "Cu",
//        "roughness" : [0.001, 0.001]
//    }
class MetalMaterial : public Material {
private:
    VS_MAKE_SLOT(eta);
    VS_MAKE_SLOT(k);
    VS_MAKE_SLOT(roughness);
    bool remapping_roughness_{false};
    float alpha_threshold_{0.022};

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<ConductorBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    MetalMaterial() = default;
    explicit MetalMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {
        roughness_.set(Slot::create_slot(desc.slot("roughness", make_float2(0.01f))));
        init_ior(desc);
        init_slot_cursor(&eta_, 3);
    }
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->input_float("alpha_threshold", &alpha_threshold_, 0.001, 0.002);
        Material::render_sub_UI(widgets);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void init_ior(const MaterialDesc &desc) noexcept {
        const ComplexIor &complex_ior = ComplexIorTable::instance()->get_ior(desc["material_name"].as_string());
        SlotDesc eta_slot;
        SlotDesc k_slot;
        if (spectrum()->is_complete()) {
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

        eta_.set(Slot::create_slot(eta_slot));
        k_.set(Slot::create_slot(k_slot));
    }

    void prepare() noexcept override {
        eta_->prepare();
        k_->prepare();
    }

    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr{swl.dimension(), 1.f};
        Float2 alpha = roughness_.evaluate(it, swl).as_vec2();
        alpha = remapping_roughness_ ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        Float alpha_min = min(alpha.x, alpha.y);
        Uint flag = select(alpha_min < alpha_threshold_, SurfaceData::NearSpec, SurfaceData::Glossy);
        SampledSpectrum eta = SampledSpectrum{eta_.evaluate(it, swl)};
        SampledSpectrum k = SampledSpectrum{k_.evaluate(it, swl)};
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelConductor>(eta, k, swl, pipeline());
        MicrofacetReflection bxdf(kr, swl, microfacet);
        return make_unique<ConductorBxDFSet>(fresnel, ocarina::move(bxdf), flag);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MetalMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-metal.dll")