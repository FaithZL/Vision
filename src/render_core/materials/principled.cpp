//
// Created by Zero on 2025/1/16.
//

#include <utility>

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {


class FresnelPrincipled : public Fresnel {
private:
    SampledSpectrum R0_;
    Float metallic_;
    Float eta_;

public:
    FresnelPrincipled(const SampledSpectrum &R0, Float metallic, const Float &eta,
                  const SampledWavelengths &swl, const Pipeline *rp)
        : Fresnel(swl, rp), R0_(R0), metallic_(metallic), eta_(eta) {}
    void correct_eta(Float cos_theta) noexcept override {
        eta_ = select(cos_theta > 0, eta_, rcp(eta_));
    }
    [[nodiscard]] SampledSpectrum evaluate(Float cos_theta) const noexcept override {
        return lerp(metallic_,
                    fresnel_dielectric(cos_theta, eta_),
                    fresnel_schlick(R0_, cos_theta));
    }
    [[nodiscard]] SampledSpectrum eta() const noexcept override { return {swl_->dimension(), eta_}; }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelPrincipled>(R0_, metallic_, eta_, *swl_, rp_);
    }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelPrincipled)
};

class PrincipledBxDFSet : public BxDFSet {
private:
    const SampledWavelengths *swl_{};
    DCSP<Fresnel> fresnel_{};
    optional<LambertReflection> diffuse_;
    optional<MicrofacetReflection> spec_refl_{};

    // sampling strategy
    static constexpr size_t max_sampling_strategy_num = 5u;
    array<Float, max_sampling_strategy_num> sampling_weights_;
    uint diffuse_index_{InvalidUI32};
    uint spec_refl_index_{InvalidUI32};
    uint clearcoat_index_{InvalidUI32};
    uint sheen_index_{};
    uint spec_trans_index_{InvalidUI32};
    uint sampling_strategy_num_{0u};

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64( diffuse_.has_value(),
                      spec_refl_.has_value(),
                      diffuse_index_, spec_refl_index_,
                      clearcoat_index_, spec_trans_index_,
                      sampling_strategy_num_);
    }

private:
    template<typename T, typename... Args>
    [[nodiscard]] SampledSpectrum lobe_f(const optional<T> &lobe, Args &&...args) const noexcept {
        if (lobe.has_value()) {
            return OC_FORWARD(lobe)->f(OC_FORWARD(args)...);
        }
        return SampledSpectrum(swl_->dimension(), 0.f);
    }

    template<typename T, typename... Args>
    [[nodiscard]] Float lobe_PDF(const optional<T> &lobe, Args &&...args) const noexcept {
        if (lobe.has_value()) {
            return OC_FORWARD(lobe)->PDF(OC_FORWARD(args)...);
        }
        return 0.f;
    }

public:
    PrincipledBxDFSet(const Interaction &it, const SampledWavelengths &swl, const Pipeline *rp, Slot color_slot,
                      Slot metallic_slot, Slot eta_slot, Slot roughness_slot,
                      Slot spec_tint_slot, Slot anisotropic_slot, Slot sheen_slot, Slot sheen_roughness_slot,
                      Slot sheen_tint_slot, Slot clearcoat_slot, Slot clearcoat_roughness_slot, Slot clearcoat_tint_slot,
                      Slot spec_trans_slot) : swl_(&swl) {
        auto [color, color_lum] = color_slot.eval_albedo_spectrum(it, swl);
        Float metallic = metallic_slot.evaluate(it, swl).as_scalar();
        diffuse_ = LambertReflection(color, swl);
        bool has_diffuse = false;
    }
    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override {
        return diffuse_->albedo(wo);
    }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode, const Uint &flag) const noexcept override {
        return diffuse_->safe_evaluate(wo, wi, nullptr, mode);
    }
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        return diffuse_->sample(wo, sampler, nullptr);
    }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        return diffuse_->sample_wi(wo, sampler->next_2d(), nullptr);
    }
};

class PrincipledMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(metallic)
    VS_MAKE_SLOT(ior)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(spec_tint)
    VS_MAKE_SLOT(anisotropic)

    VS_MAKE_SLOT(sheen_weight)
    VS_MAKE_SLOT(sheen_roughness)
    VS_MAKE_SLOT(sheen_tint)

    VS_MAKE_SLOT(clearcoat_weight)
    VS_MAKE_SLOT(clearcoat_roughness)
    VS_MAKE_SLOT(clearcoat_tint)

    VS_MAKE_SLOT(subsurface_weight)
    VS_MAKE_SLOT(subsurface_radius)
    VS_MAKE_SLOT(subsurface_scale)

    VS_MAKE_SLOT(spec_trans)

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<PrincipledBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    PrincipledMaterial() = default;
    explicit PrincipledMaterial(const MaterialDesc &desc)
        : Material(desc) {

#define INIT_SLOT(name, default_value, type) \
    name##_.set(Slot::create_slot(desc.slot(#name, default_value, type)));

        INIT_SLOT(color, make_float3(1.f), Albedo);
        INIT_SLOT(metallic, 0.f, Number);
        INIT_SLOT(ior, 1.5f, Number);
        INIT_SLOT(roughness, 0.5f, Number);
        INIT_SLOT(spec_tint, make_float3(0.f), Albedo);
        INIT_SLOT(anisotropic, 0.f, Number);

        INIT_SLOT(sheen_weight, 0.f, Number);
        INIT_SLOT(sheen_roughness, 0.5f, Number);
        INIT_SLOT(sheen_tint, make_float3(1.f), Albedo);

        INIT_SLOT(clearcoat_weight, 0.3f, Number);
        INIT_SLOT(clearcoat_roughness, 0.2f, Number);
        INIT_SLOT(clearcoat_tint, make_float3(1.f), Albedo);

        INIT_SLOT(subsurface_weight, 0.3f, Number);
        INIT_SLOT(subsurface_radius, make_float3(1.f), Number);
        INIT_SLOT(subsurface_scale, 0.2f, Number);

        INIT_SLOT(spec_trans, 0.f, Number);

#undef INIT_SLOT
        init_slot_cursor(&color_, &spec_trans_);
    }
    void restore(RuntimeObject *old_obj) noexcept override {
        Material::restore(old_obj);
    }
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        return make_unique<PrincipledBxDFSet>(it, swl, pipeline(), color_, metallic_,
                                              ior_, roughness_, spec_tint_, anisotropic_,
                                              sheen_weight_, sheen_roughness_, sheen_tint_,
                                              clearcoat_weight_,clearcoat_roughness_,
                                              clearcoat_tint_,spec_trans_);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, PrincipledMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-principled.dll")