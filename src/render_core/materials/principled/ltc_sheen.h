//
// Created by ling.zhu on 2025/1/30.
//

#pragma once

#include "base/scattering/material.h"
#include "base/scattering/bxdf_set.h"

namespace vision {

struct SheenLTCTable {
private:
    Texture approx_;
    Texture volume_;
    static constexpr auto res = 32;

private:
    static SheenLTCTable *s_sheen_table;
    SheenLTCTable() = default;

public:
    SheenLTCTable(const SheenLTCTable &) = delete;
    SheenLTCTable(SheenLTCTable &&) = delete;
    SheenLTCTable operator=(const SheenLTCTable &) = delete;
    SheenLTCTable operator=(SheenLTCTable &&) = delete;

public:
    static SheenLTCTable &instance();
    static void destroy_instance();
    void init() noexcept;
    [[nodiscard]] Float4 sample_approx(const Float &cos_theta, const Float &alpha) noexcept;
    [[nodiscard]] Float4 sample_volume(const Float &cos_theta, const Float &alpha) noexcept;
};

class SheenLTC : public BxDFSet {
protected:
    const SampledWavelengths *swl_{nullptr};
    SampledSpectrum tint_;
    Float alpha_;
    Float4 c_;

public:
    SheenLTC(const Float &cos_theta, SampledSpectrum tint, Float alpha, const SampledWavelengths &swl)
        : tint_(std::move(tint)), alpha_(std::move(alpha)), swl_(&swl) {
        c_ = SheenLTCTable::instance().sample_approx(cos_theta, alpha_);
    }
    [[nodiscard]] SampledSpectrum albedo(const ocarina::Float3 &wo) const noexcept override { return tint_;}
    [[nodiscard]] Uint flag() const noexcept override { return BxDFFlag::GlossyRefl; }
    [[nodiscard]] static Float3 rotate(const Float3 &v, const Float3 &axis,
                                       const Float &angle) noexcept;
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode, const Uint &flag) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;
    [[nodiscard]] Float3 sample_ltc(const Float2 &u) const noexcept;
    [[nodiscard]] Float eval_ltc(const Float3 &wi) const noexcept;
    [[nodiscard]] const SampledWavelengths *swl() const override { return swl_; }
};

}// namespace vision