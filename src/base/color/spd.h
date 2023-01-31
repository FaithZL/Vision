//
// Created by Zero on 26/12/2022.
//

#pragma once

#include "dsl/common.h"
#include "rhi/common.h"
#include "base/mgr/render_pipeline.h"
#include "cie.h"
#include "spectrum.h"

namespace vision {

using namespace ocarina;

/**
 * Cauchy's dispersion formula
 * eta(lambda) = a + b / (lambda^2) + c / (lambda^4)
 */
struct CauchyDispersion {
public:
    float a;
    float b;
    float c;

private:
    explicit CauchyDispersion(float3 eta_rgb, float3 lambdas = rgb_spectrum_peak_wavelengths) {
        // todo
    }

    template<EPort p = D>
    [[nodiscard]] oc_float<p> eta(const oc_float<p> &lambda) const noexcept {
        return a + b / sqr(lambda) + c / Pow<4>(lambda);
    }
};

class SPD {
private:
    ManagedWrapper<float> _func;
    float _sample_interval{};
    RenderPipeline *_rp{};

public:
    explicit SPD(RenderPipeline *rp);
    SPD(vector<float> func, RenderPipeline *rp);
    void init(vector<float> func) noexcept;
    void prepare() noexcept;
    [[nodiscard]] Float sample(const Float& lambda) const noexcept;
    [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] static SPD create_cie_x(RenderPipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_y(RenderPipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_z(RenderPipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_d65(RenderPipeline *rp) noexcept;
    [[nodiscard]] static float cie_y_integral() noexcept;
};

}// namespace vision