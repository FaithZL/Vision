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
class CauchyDispersion {
private:
    float _a;
    float _b;
    float _c;

public:
    CauchyDispersion(float a, float b, float c) noexcept
        : _a(a), _b(b), _c(c) {}

    explicit CauchyDispersion(float3 eta,
                              float3 lambdas = rgb_spectrum_peak_wavelengths) noexcept {
//        float a1 = 1, b1 = 1 / sqr(lambdas[0]), c1 = 1 / Pow<4>(lambdas[0]), d1 = eta[0];
//        float a2 = 1, b2 = 1 / sqr(lambdas[1]), c2 = 1 / Pow<4>(lambdas[1]), d2 = eta[1];
//        float a3 = 1, b3 = 1 / sqr(lambdas[2]), c3 = 1 / Pow<4>(lambdas[2]), d3 = eta[2];
//
//        float Dx = det(make_float3x3(d1, d2, d3,
//                                     b1, b2, b3,
//                                     c1, c2, c3));
//
//        float Dy = det(make_float3x3(a1, a2, a3,
//                                     d1, d2, d3,
//                                     c1, c2, c3));
//
//        float Dz = det(make_float3x3(a1, a2, a3,
//                                     b1, b2, b3,
//                                     d1, d2, d3));
//
//        float D = det(make_float3x3(a1, a2, a3,
//                                    b1, b2, b3,
//                                    c1, c2, c3));
//
//        _a = Dx / D;
//        _b = Dy / D;
//        _c = Dz / D;
    }

    [[nodiscard]] auto eta(const auto &lambda) const noexcept {
        return _a + _b / sqr(lambda) + _c / Pow<4>(lambda);
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