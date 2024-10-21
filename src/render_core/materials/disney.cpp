//
// Created by Zero on 28/10/2022.
//

#include <utility>

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

inline namespace disney {

class Diffuse : public BxDF {
private:
    SampledSpectrum color_;

public:
    Diffuse() = default;
    explicit Diffuse(SampledSpectrum color, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl),
          color_(std::move(color)) {}
    // clang-format off
    VS_MAKE_BxDF_ASSIGNMENT(Diffuse)
        // clang-format on
        [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return color_; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wi) {
            Float Fo = schlick_weight(abs_cos_theta(wo));
            Float Fi = schlick_weight(abs_cos_theta(wi));
            return InvPi * (1 - 0.5f * Fo) * (1 - 0.5f * Fi);
        };
        impl.function()->set_description("disney::Diffuse::f");
        return color_ * impl(wi, wi);
    }
};

class FakeSS : public BxDF {
private:
    SampledSpectrum color_;
    Float roughness_;

public:
    FakeSS() = default;
    explicit FakeSS(SampledSpectrum color, Float r, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl),
          color_(color),
          roughness_(r) {}
    // clang-format off
    VS_MAKE_BxDF_ASSIGNMENT(FakeSS)
        // clang-format on
        [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return color_; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wi, Float roughness) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float cos_theta_d = dot(wi, wh);
            Float Fss90 = sqr(cos_theta_d) * roughness;
            Float Fo = schlick_weight(abs_cos_theta(wo));
            Float Fi = schlick_weight(abs_cos_theta(wi));
            Float Fss = lerp(Fo, 1.f, Fss90) * lerp(Fi, 1.f, Fss90);
            Float ss = 1.25f * (Fss * (1 / (abs_cos_theta(wo) + abs_cos_theta(wi)) - .5f) + .5f);
            Float ret = InvPi * ss;
            return select(valid, ret, 0.f);
        };
        impl.function()->set_description("disney::FakeSS::f");
        return color_ * impl(wo, wi, roughness_);
    }
};

class Retro : public BxDF {
private:
    SampledSpectrum color_;
    Float roughness_;

public:
    VS_MAKE_BxDF_ASSIGNMENT(Retro)
        Retro() = default;
    explicit Retro(SampledSpectrum color, Float r, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl),
          color_(color),
          roughness_(r) {}
    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return color_; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wi, Float roughness) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float cos_theta_d = dot(wi, wh);

            Float Fo = schlick_weight(abs_cos_theta(wo));
            Float Fi = schlick_weight(abs_cos_theta(wi));
            Float Rr = 2 * roughness * sqr(cos_theta_d);

            Float ret = InvPi * Rr * (Fo + Fi + Fo * Fi * (Rr - 1));
            return select(valid, ret, 0.f);
        };
        impl.function()->set_description("disney::Retro::f");
        return color_ * impl(wo, wi, roughness_);
    }
};

class Sheen : public BxDF {
private:
    SampledSpectrum color_;

public:
    VS_MAKE_BxDF_ASSIGNMENT(Sheen)
        Sheen() = default;
    explicit Sheen(SampledSpectrum kr, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl),
          color_(kr) {}
    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return color_; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wi) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float cos_theta_d = dot(wi, wh);
            Float ret = schlick_weight(cos_theta_d);
            return select(valid, ret, 0.f);
        };
        impl.function()->set_description("disney::Sheen::f");
        return color_ * impl(wo, wi);
    }
};

[[nodiscard]] Float GTR1(Float cos_theta, Float alpha) {
    static CALLABLE_TYPE impl = [](Float cos_theta, Float alpha) {
        Float alpha2 = sqr(alpha);
        return (alpha2 - 1) /
               (Pi * log(alpha2) * (1 + (alpha2 - 1) * sqr(cos_theta)));
    };
    impl.function()->set_description("disney::GTR1");
    return impl(cos_theta, alpha);
}

[[nodiscard]] Float smithG_GGX(Float cos_theta, Float alpha) {
    static CALLABLE_TYPE impl = [](Float cos_theta, Float alpha) {
        Float alpha2 = sqr(alpha);
        Float cos_theta_2 = sqr(cos_theta);
        return 1 / (cos_theta + sqrt(alpha2 + cos_theta_2 - alpha2 * cos_theta_2));
    };
    impl.function()->set_description("disney::smithG_GGX");
    return impl(cos_theta, alpha);
}

class Clearcoat : public BxDF {
private:
    Float weight_;
    Float alpha_;

public:
    VS_MAKE_BxDF_ASSIGNMENT(Clearcoat)
        Clearcoat() = default;
    Clearcoat(Float weight, Float alpha, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::GlossyRefl),
          weight_(weight),
          alpha_(alpha) {}
    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return {swl().dimension(), weight_}; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wi, Float weight, Float alpha) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float Dr = GTR1(abs_cos_theta(wh), alpha);
            Float Fr = fresnel_schlick<D>(0.04f, dot(wo, wh));
            Float Gr = smithG_GGX(abs_cos_theta(wo), 0.25f) * smithG_GGX(abs_cos_theta(wi), 0.25f);
            Float ret = weight * Gr * Fr * Dr * 0.25f;
            return select(valid, ret, 0.f);
        };
        impl.function()->set_description("disney::Clearcoat::f");
        return {swl().dimension(), impl(wo, wi, weight_, alpha_)};
    }
    [[nodiscard]] Float PDF(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wi, Float alpha) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float Dr = GTR1(abs_cos_theta(wh), alpha);
            Float ret = Dr * abs_cos_theta(wh) / (4 * dot(wo, wh));
            return select(valid, ret, 0.f);
        };
        impl.function()->set_description("disney::Clearcoat::PDF");
        return impl(wo, wi, alpha_);
    }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, Float2 u, SP<Fresnel> fresnel) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, Float2 u, Float alpha) {
            Float alpha2 = sqr(alpha);
            Float cos_theta = safe_sqrt((1 - pow(alpha2, 1 - u[0])) / (1 - alpha2));
            Float sin_theta = safe_sqrt(1 - sqr(cos_theta));
            Float phi = 2 * Pi * u[1];
            Float3 wh = spherical_direction(sin_theta, cos_theta, phi);
            wh = select(same_hemisphere(wo, wh), wh, -wh);
            return reflect(wo, wh);
        };
        impl.function()->set_description("disney::Clearcoat::sample_wi");
        Float3 wi = impl(wo, u, alpha_);
        return {wi, 1.f};
    }

    [[nodiscard]] BSDFSample sample(const Float3 &wo, TSampler &sampler, SP<Fresnel> fresnel) const noexcept override {
        Float2 u = sampler->next_2d();
        auto [wi, pdf] = sample_wi(wo, u, fresnel);
        BSDFSample ret{swl()};
        ret.eval = safe_evaluate(wo, wi, nullptr, MaterialEvalMode::All);
        ret.wi = wi;
        return ret;
    }
};

class FresnelDisney : public Fresnel {
private:
    SampledSpectrum R0_;
    Float metallic_;
    Float eta_;

public:
    FresnelDisney(const SampledSpectrum &R0, Float metallic, const Float &eta,
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
        return make_shared<FresnelDisney>(R0_, metallic_, eta_, *swl_, rp_);
    }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelDisney)
};

}// namespace disney

class DisneyMicrofacet : public Microfacet<D> {
private:
    static constexpr MicrofacetType type = MicrofacetType::Disney;
    using Super = Microfacet<D>;

public:
    explicit DisneyMicrofacet(Float2 alpha) : Super(alpha, type) {}
    DisneyMicrofacet(Float ax, Float ay) : Super(ax, ay, type) {}

    [[nodiscard]] Float D_(Float3 wh) const noexcept override {
        static CALLABLE_TYPE impl = [](Float3 wh, Float ax, Float ay) {
            return microfacet::D_<D>(wh, ax, ay, type);
        };
        impl.function()->set_description("disney::DisneyMicrofacet::D");
        return impl(wh, alpha_x_, alpha_y_);
    }
    [[nodiscard]] Float3 sample_wh(const Float3 &wo, const Float2 &u) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, Float2 u, Float ax, Float ay) {
            return microfacet::sample_wh<D>(wo, u, ax, ay, type);
        };
        impl.function()->set_description("disney::DisneyMicrofacet::sample_wh");
        return impl(wo, u, alpha_x_, alpha_y_);
    }
    [[nodiscard]] Float PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh, Float ax, Float ay) {
            return microfacet::PDF_wh<D>(wo, wh, ax, ay, type);
        };
        impl.function()->set_description("disney::DisneyMicrofacet::PDF_wh");
        return impl(wo, wh, alpha_x_, alpha_y_);
    }

    [[nodiscard]] Float PDF_wi_reflection(const Float &pdf_wh, const Float3 &wo, const Float3 &wh) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float &pdf_wh, const Float3 &wo, const Float3 &wh) {
            return microfacet::PDF_wi_reflection<D>(pdf_wh, wo, wh);
        };
        impl.function()->set_description("disney::DisneyMicrofacet::PDF_wi_reflection");
        return impl(pdf_wh, wo, wh);
    }

    [[nodiscard]] Float PDF_wi_reflection(const Float3 &wo, const Float3 &wh) const noexcept override {
        return PDF_wi_reflection(PDF_wh(wo, wh), wo, wh);
    }

    [[nodiscard]] Float PDF_wi_transmission(const Float &pdf_wh, const Float3 &wo, const Float3 &wh,
                                            const Float3 &wi, const Float &eta) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float &pdf_wh, const Float3 &wo, const Float3 &wh, const Float3 &wi, const Float &eta) {
            return microfacet::PDF_wi_transmission<D>(pdf_wh, wo, wh, wi, eta);
        };
        impl.function()->set_description("disney::DisneyMicrofacet::PDF_wi_transmission");
        return impl(pdf_wh, wo, wh, wi, eta);
    }

    [[nodiscard]] Float PDF_wi_transmission(const Float3 &wo, const Float3 &wh, const Float3 &wi, const Float &eta) const noexcept override {
        return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, eta);
    }

    [[nodiscard]] SampledSpectrum BRDF(const Float3 &wo, const Float3 &wh, const Float3 &wi, const SampledSpectrum &Fr) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh, const Float3 &wi, Float ax, Float ay) {
            return microfacet::BRDF_div_fr<D>(wo, wh, wi, ax, ay, type);
        };
        impl.function()->set_description("disney::DisneyMicrofacet::BRDF_div_fr");
        return impl(wo, wh, wi, alpha_x_, alpha_y_) * Fr;
    }

    [[nodiscard]] SampledSpectrum BRDF(const Float3 &wo, const Float3 &wi, const SampledSpectrum &Fr) const noexcept override {
        Float3 wh = normalize(wo + wi);
        return BRDF(wo, wh, wi, Fr);
    }

    [[nodiscard]] SampledSpectrum BTDF(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                       const SampledSpectrum &Ft, const Float &eta) const noexcept override {
        static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh, const Float3 &wi, const Float &eta, Float ax, Float ay) {
            return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
        };
        impl.function()->set_description("disney::DisneyMicrofacet::BTDF_div_ft");
        return impl(wo, wh, wi, eta, alpha_x_, alpha_y_) * Ft;
    }

    [[nodiscard]] SampledSpectrum BTDF(const Float3 &wo, const Float3 &wi, const SampledSpectrum &Ft, const Float &eta) const noexcept override {
        Float3 wh = normalize(wo + wi * eta);
        return BTDF(wo, wh, wi, Ft, eta);
    }
};

class PrincipledBxDFSet : public BxDFSet {
private:
    const SampledWavelengths *swl_{};
    DCSP<Fresnel> fresnel_{};
    optional<Diffuse> diffuse_{};
    optional<Retro> retro_{};
    optional<Sheen> sheen_{};
    optional<FakeSS> fake_ss_{};
    optional<MicrofacetReflection> spec_refl_{};
    optional<Clearcoat> clearcoat_{};
    optional<MicrofacetTransmission> spec_trans_{};

    // sampling strategy
    static constexpr size_t max_sampling_strategy_num = 4u;
    array<Float, max_sampling_strategy_num> sampling_weights_;
    uint diffuse_index_{InvalidUI32};
    uint spec_refl_index_{InvalidUI32};
    uint clearcoat_index_{InvalidUI32};
    uint spec_trans_index_{InvalidUI32};
    uint sampling_strategy_num_{0u};

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(fresnel_->type_hash(), diffuse_.has_value(),
                      retro_.has_value(), sheen_.has_value(),
                      fake_ss_.has_value(), spec_refl_.has_value(),
                      clearcoat_.has_value(), spec_trans_.has_value(),
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

    template<typename... Args>
    [[nodiscard]] SampledSpectrum f_diffuse(Args &&...args) const noexcept {
        return lobe_f(diffuse_, OC_FORWARD(args)...) + lobe_f(retro_, OC_FORWARD(args)...) +
               lobe_f(sheen_, OC_FORWARD(args)...) + lobe_f(fake_ss_, OC_FORWARD(args)...);
    }

    template<typename... Args>
    [[nodiscard]] Float PDF_diffuse(Args &&...args) const noexcept {
        return lobe_PDF(diffuse_, OC_FORWARD(args)...);
    }

public:
    PrincipledBxDFSet(const Interaction &it, const SampledWavelengths &swl, const Pipeline *rp, Slot color_slot,
                      Slot metallic_slot, Slot eta_slot, Slot roughness_slot,
                      Slot spec_tint_slot, Slot anisotropic_slot, Slot sheen_slot,
                      Slot sheen_tint_slot, Slot clearcoat_slot, Slot clearcoat_alpha_slot,
                      Slot spec_trans_slot, Slot flatness_slot, Slot diff_trans_slot) : swl_(&swl) {

        auto [color, color_lum] = color_slot.eval_albedo_spectrum(it, swl);
        Float metallic = metallic_slot.evaluate(it, swl).as_scalar();
        Float spec_trans = spec_trans_slot.evaluate(it, swl).as_scalar();
        Float diffuse_weight = (1.f - metallic) * (1 - spec_trans);
        Float flatness = flatness_slot.evaluate(it, swl).as_scalar();
        Float roughness = roughness_slot.evaluate(it, swl).as_scalar();
        Float tint_weight = select(color_lum > 0.f, 1.f / color_lum, 1.f);
        SampledSpectrum tint = clamp(color * tint_weight, 0.f, 1.f);
        Float tint_lum = color_lum * tint_weight;

        Float Cdiff_weight = diffuse_weight * (1.f - flatness);
        SampledSpectrum Cdiff = color * Cdiff_weight;

        bool has_diffuse = false;

        if (!color_slot->is_zero()) {
            diffuse_ = Diffuse(Cdiff, swl);
            retro_ = Retro(Cdiff, roughness, swl);
            has_diffuse = true;
        }

        if (!flatness_slot->is_zero()) {
            Float Css_weight = diffuse_weight * flatness;
            SampledSpectrum Css = Css_weight * color;
            fake_ss_ = FakeSS(Css, roughness, swl);
            has_diffuse = true;
        }

        if (!sheen_slot->is_zero()) {
            Float sheen = sheen_slot.evaluate(it, swl).as_scalar();
            Float sheen_tint = sheen_tint_slot.evaluate(it, swl).as_scalar();
            Float Csheen_weight = diffuse_weight * sheen;
            SampledSpectrum Csheen = Csheen_weight * lerp(sheen_tint, 1.f, tint);
            sheen_ = Sheen(Csheen, swl);
            has_diffuse = true;
        }

        if (has_diffuse) {
            diffuse_index_ = sampling_strategy_num_++;
            sampling_weights_[diffuse_index_] = saturate(diffuse_weight * color_lum);
        }

        Float spec_tint = spec_tint_slot.evaluate(it, swl).as_scalar();
        Float eta = eta_slot.evaluate(it, swl).as_scalar();
        Float SchlickR0 = schlick_R0_from_eta(eta);
        SampledSpectrum Cspec0 = lerp(metallic, lerp(spec_tint, 1.f, tint) * SchlickR0, color);

        fresnel_ = make_deep_copy_shared<FresnelDisney>(Cspec0, metallic, eta, swl, rp);
        Float anisotropic = anisotropic_slot.evaluate(it, swl).as_scalar();
        Float aspect = sqrt(1 - anisotropic * 0.9f);
        Float2 alpha = make_float2(max(0.001f, sqr(roughness) / aspect),
                                   max(0.001f, sqr(roughness) * aspect));
        auto microfacet = make_shared<DisneyMicrofacet>(alpha);
        spec_refl_ = MicrofacetReflection(SampledSpectrum(swl.dimension(), 1.f), swl, microfacet);
        Float Cspec0_lum = lerp(metallic, lerp(spec_tint, 1.f, tint_lum) * SchlickR0, color_lum);
        spec_refl_index_ = sampling_strategy_num_++;
        sampling_weights_[spec_refl_index_] = saturate(Cspec0_lum);

        if (!clearcoat_slot->is_zero()) {
            Float cc = clearcoat_slot.evaluate(it, swl).as_scalar();
            Float cc_alpha = lerp(clearcoat_alpha_slot.evaluate(it, swl).as_scalar(), 0.001f, 1.f);
            clearcoat_ = Clearcoat(cc, cc_alpha, swl);
            clearcoat_index_ = sampling_strategy_num_++;
            sampling_weights_[clearcoat_index_] = saturate(cc * fresnel_schlick(0.04f, 1.f));
        }

        if (!spec_trans_slot->is_zero()) {
            Float Cst_weight = (1.f - metallic) * spec_trans;
            SampledSpectrum Cst = Cst_weight * sqrt(color);
            spec_trans_ = MicrofacetTransmission(Cst, swl, microfacet);
            Float Cst_lum = Cst_weight * sqrt(color_lum);
            spec_trans_index_ = sampling_strategy_num_++;
            sampling_weights_[spec_trans_index_] = saturate(Cst_lum);
        }

        Float sum_weights = 0.f;
        for (uint i = 0u; i < sampling_strategy_num_; i++) {
            sum_weights += sampling_weights_[i];
        }
        Float inv_sum_weights = select(sum_weights == 0.f, 0.f, 1.f / sum_weights);
        for (uint i = 0u; i < sampling_strategy_num_; i++) {
            sampling_weights_[i] *= inv_sum_weights;
        }
    }
    VS_MAKE_BxDFSet_ASSIGNMENT(PrincipledBxDFSet)

        [[nodiscard]] Bool splittable() const noexcept override {
        if (!spec_trans_) {
            return false;
        }
        return true;
    }
    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return diffuse_->albedo(wo); }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3& wi, MaterialEvalMode mode, const Uint &flag) const noexcept override {
        return outline([&] {
            ScatterEval ret{spec_refl_->swl()};
            SampledSpectrum f = {spec_refl_->swl().dimension(), 0.f};
            Float pdf = 0.f;
            auto fresnel = fresnel_->clone();
            Float cos_theta_o = cos_theta(wo);
            fresnel->correct_eta(cos_theta_o);
            $if(same_hemisphere(wo, wi)) {
                if (diffuse_index_ != InvalidUI32) {
                    if (BxDF::match_F(mode)) {
                        f = f_diffuse(wo, wi, fresnel);
                    }
                    ret.flags = BxDFFlag::DiffRefl;
                    if (BxDF::match_PDF(mode)) {
                        pdf = sampling_weights_[diffuse_index_] * PDF_diffuse(wo, wi, fresnel);
                    }
                }
                if (BxDF::match_F(mode)) {
                    f += spec_refl_->f(wo, wi, fresnel);
                }
                if (BxDF::match_PDF(mode)) {
                    pdf += sampling_weights_[spec_refl_index_] * spec_refl_->PDF(wo, wi, fresnel);
                }
                ret.flags = BxDFFlag::GlossyRefl;
                if (clearcoat_.has_value()) {
                    if (BxDF::match_F(mode)) {
                        f += clearcoat_->f(wo, wi, fresnel);
                    }
                    if (BxDF::match_PDF(mode)) {
                        pdf += sampling_weights_[clearcoat_index_] * clearcoat_->PDF(wo, wi, fresnel);
                    }
                }
            }
            $else {
                if (spec_trans_.has_value()) {
                    if (BxDF::match_F(mode)) {
                        f = spec_trans_->f(wo, wi, fresnel);
                    }
                    ret.flags = BxDFFlag::GlossyTrans;
                    if (BxDF::match_PDF(mode)) {
                        pdf = sampling_weights_[spec_trans_index_] * spec_trans_->PDF(wo, wi, fresnel);
                    }
                }
            };
            ret.f = f;
            ret.pdfs =pdf;
            return ret;
        },
                       "PrincipledBxDFSet::evaluate_local");
    }

    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        return outline([&] {
            Float uc = sampler->next_1d();
            Float2 u = sampler->next_2d();
            SampledDirection sampled_direction;

            Uint sampling_strategy = 0u;
            Float sum_weights = 0.f;
            for (uint i = 0; i < sampling_strategy_num_; ++i) {
                sampling_strategy = select(uc > sum_weights, i, sampling_strategy);
                sum_weights += sampling_weights_[i];
            }
            auto fresnel = fresnel_->clone();
            Float cos_theta_o = cos_theta(wo);
            fresnel->correct_eta(cos_theta_o);

            $switch(sampling_strategy) {
                if (diffuse_.has_value()) {
                    $case(diffuse_index_) {
                        sampled_direction = diffuse_->sample_wi(wo, u, fresnel);
                        $break;
                    };
                }
                $case(spec_refl_index_) {
                    sampled_direction = spec_refl_->sample_wi(wo, u, fresnel);
                    $break;
                };
                if (clearcoat_.has_value()) {
                    $case(clearcoat_index_) {
                        sampled_direction = clearcoat_->sample_wi(wo, u, fresnel);
                        $break;
                    };
                }
                if (spec_trans_.has_value()) {
                    $case(spec_trans_index_) {
                        sampled_direction = spec_trans_->sample_wi(wo, u, fresnel);
                        $break;
                    };
                }
                $default {
                    unreachable();
                    $break;
                };
            };
            return sampled_direction;
        },
                       "PrincipledBxDFSet::sample_wi");
    }

    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        BSDFSample ret{spec_refl_->swl()};
        SampledDirection sampled_direction = sample_wi(wo, flag, sampler);
        ret.eval = evaluate_local(wo, sampled_direction.wi, MaterialEvalMode::All, flag);
        ret.wi = sampled_direction.wi;
        ret.eval.pdfs =select(sampled_direction.valid(), ret.eval.pdf() * sampled_direction.pdf, 0.f);
        return ret;
    }
};

//    "type" : "disney",
//    "param" : {
//        "color" : [
//            1,1,1
//        ],
//        "ior" : 1.5,
//        "roughness" : 0.1,
//        "metallic" : 1,
//        "spec_tint" : 0.0,
//        "anisotropic" : 0,
//        "sheen" : 0,
//        "sheen_tint" : 0,
//        "clearcoat" : 0,
//        "clearcoat_alpha" : 0.1,
//        "spec_trans" : 1,
//        "flatness" : 0.98,
//        "scatter_distance" : [0, 0, 0],
//        "diff_trans" : 0.11
//    }
class DisneyMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(metallic)
    VS_MAKE_SLOT(eta)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(spec_tint)
    VS_MAKE_SLOT(anisotropic)
    VS_MAKE_SLOT(sheen)
    VS_MAKE_SLOT(sheen_tint)
    VS_MAKE_SLOT(clearcoat)
    VS_MAKE_SLOT(clearcoat_alpha)
    VS_MAKE_SLOT(spec_trans)
    VS_MAKE_SLOT(flatness)
    VS_MAKE_SLOT(diff_trans)
    bool thin_{false};

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<PrincipledBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    DisneyMaterial() = default;
    explicit DisneyMaterial(const MaterialDesc &desc)
        : Material(desc) {
        color_.set(Slot::create_slot(desc.slot("color", make_float3(1.f), Albedo)));
        metallic_.set(Slot::create_slot(desc.slot("metallic", 0.f, Number)));
        eta_.set(Slot::create_slot(desc.slot("ior", 1.5f, Number)));
        roughness_.set(Slot::create_slot(desc.slot("roughness", 0.5f, Number)));
        spec_tint_.set(Slot::create_slot(desc.slot("spec_tint", 0.f, Number)));
        anisotropic_.set(Slot::create_slot(desc.slot("anisotropic", 0.f, Number)));
        sheen_.set(Slot::create_slot(desc.slot("sheen", 0.f, Number)));
        sheen_tint_.set(Slot::create_slot(desc.slot("sheen_tint", 0.f, Number)));
        clearcoat_.set(Slot::create_slot(desc.slot("clearcoat", 0.3f, Number)));
        clearcoat_alpha_.set(Slot::create_slot(desc.slot("clearcoat_alpha", 0.2f, Number)));
        spec_trans_.set(Slot::create_slot(desc.slot("spec_trans", 0.f, Number)));
        flatness_.set(Slot::create_slot(desc.slot("flatness", 0.f, Number)));
        diff_trans_.set(Slot::create_slot(desc.slot("diff_trans", 0.f, Number)));
        init_slot_cursor(&color_, &diff_trans_);
    }
    void restore(RuntimeObject *old_obj) noexcept override {
        Material::restore(old_obj);
        VS_HOTFIX_MOVE_ATTRS(thin_)
    }
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        return make_unique<PrincipledBxDFSet>(it, swl, pipeline(), color_, metallic_,
                                              eta_, roughness_, spec_tint_, anisotropic_,
                                              sheen_, sheen_tint_, clearcoat_, clearcoat_alpha_,
                                              spec_trans_, flatness_, diff_trans_);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, DisneyMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-disney.dll")