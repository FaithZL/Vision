//
// Created by Zero on 09/09/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class OrenNayar : public BxDF {
private:
    SampledSpectrum R_;
    Float A_, B_;

public:
    OrenNayar(SampledSpectrum R, Float sigma, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl), R_(R) {
        sigma = radians(sigma);
        Float sigma2 = ocarina::sqr(sigma * sigma);
        A_ = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
        B_ = 0.45f * sigma2 / (sigma2 + 0.09f);
    }
    VS_MAKE_BxDF_ASSIGNMENT(OrenNayar)
        [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return R_; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept override {
        Float sin_theta_i = sin_theta(wi);
        Float sin_theta_o = sin_theta(wo);

        Float sin_phi_i = sin_phi(wi);
        Float cos_phi_i = cos_phi(wi);
        Float sin_phi_o = sin_phi(wo);
        Float cos_phi_o = cos_phi(wo);
        Float d_cos = cos_phi_i * cos_phi_o + sin_phi_i * sin_phi_o;

        Float max_cos = ocarina::max(0.f, d_cos);

        Bool cond = abs_cos_theta(wi) > abs_cos_theta(wo);
        Float sin_alpha = select(cond, sin_theta_o, sin_theta_i);
        Float tan_beta = select(cond, sin_theta_i / abs_cos_theta(wi),
                                sin_theta_o / abs_cos_theta(wo));

        return R_ * InvPi * (A_ + B_ * max_cos * sin_alpha * tan_beta);
    }
};

class MatteBxDFSet : public BxDFSet {
private:
    DCUP<BxDF> bxdf_;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return bxdf_->type_hash();
    }

public:
    MatteBxDFSet(const SampledSpectrum &kr, const SampledWavelengths &swl)
        : bxdf_(std::make_unique<LambertReflection>(kr, swl)) {}
    MatteBxDFSet(SampledSpectrum R, Float sigma, const SampledWavelengths &swl)
        : bxdf_(std::make_unique<OrenNayar>(R, sigma, swl)) {}

    // clang-format off
    VS_MAKE_BxDFSet_ASSIGNMENT(MatteBxDFSet)
        // clang-format on

        [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return bxdf_->albedo(wo); }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3& wi, MaterialEvalMode mode, const Uint &flag) const noexcept override {
        return bxdf_->safe_evaluate(wo, wi, nullptr, mode);
    }
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        return bxdf_->sample(wo, sampler, nullptr);
    }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        return bxdf_->sample_wi(wo, sampler->next_2d(), nullptr);
    }
};

class MatteMaterial : public Material {
private:
    VS_MAKE_SLOT(color);
    VS_MAKE_SLOT(sigma);

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<MatteBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = color_.eval_albedo_spectrum(it, swl).sample;
        if (sigma_) {
            Float sigma = sigma_.evaluate(it, swl).as_scalar();
            return make_unique<MatteBxDFSet>(kr, sigma, swl);
        }
        return make_unique<MatteBxDFSet>(kr, swl);
    }
    MatteMaterial() = default;
    [[nodiscard]] bool enable_delta() const noexcept override { return false; }
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        Material::render_UI(widgets);
        return true;
    }
    explicit MatteMaterial(const MaterialDesc &desc)
        : Material(desc) {
        color_.set(Slot::create_slot(desc.slot("color", make_float3(0.5f), Albedo)));
        init_slot_cursor(&color_, 2);
        if (desc.has_attr("sigma")) {
            sigma_.set(Slot::create_slot(desc.slot("sigma", 1.f, Number)));
        }
    }
    VS_MAKE_PLUGIN_NAME_FUNC
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MatteMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-matte.dll")