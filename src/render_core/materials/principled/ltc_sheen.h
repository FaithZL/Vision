//
// Created by ling.zhu on 2025/1/30.
//

#pragma once

#include "ltc_sheen_table.inl.h"
#include "base/scattering/material.h"
#include "base/scattering/bxdf_set.h"
//#include "base/sample.h"

namespace vision {

struct SheenLTCTable {
private:
    Texture approx_;
    Texture volume_;

private:
    static SheenLTCTable *s_mesh_registry;
    SheenLTCTable() = default;
    SheenLTCTable(const SheenLTCTable &) = delete;
    SheenLTCTable(SheenLTCTable &&) = delete;
    SheenLTCTable operator=(const SheenLTCTable &) = delete;
    SheenLTCTable operator=(SheenLTCTable &&) = delete;

public:
    static SheenLTCTable &instance();
    static void destroy_instance();
    void init() noexcept;
};

class SheenLTC : public BxDFSet {
protected:
    const SampledWavelengths *swl_{nullptr};
    SampledSpectrum tint_;
    Float alpha_;

public:
    SheenLTC(SampledSpectrum tint, Float alpha, const SampledWavelengths &swl)
        : tint_(std::move(tint)), alpha_(std::move(alpha)), swl_(&swl) {}
    [[nodiscard]] Uint flag() const noexcept override { return BxDFFlag::GlossyRefl; }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode, const Uint &flag) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override { return swl_; }
};

}// namespace vision