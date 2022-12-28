//
// Created by Zero on 26/12/2022.
//

#include "spd.h"
#include "cie.h"

namespace vision {

using namespace ocarina;
static constexpr auto spd_lut_interval = 5u;
namespace detail {

[[nodiscard]] float densely_sampled_spectrum_integral(uint t, const float *spec) noexcept {
    float sum = 0.0;
    float tt = static_cast<float>(t);
    float n = (cie::visible_wavelength_max - cie::visible_wavelength_min) / tt;
    uint nn = static_cast<uint>(n) + 1u;
    for (uint i = 0u; i < nn - 1u; i++) {
        sum += 0.5f * (cie::Y[i * t] + cie::Y[(i + 1u) * t]);
    }
    return static_cast<float>(sum * tt);
}

[[nodiscard]] vector<float> downsample_densely_sampled_spectrum(uint interval, const float *data) noexcept {
    float factor = (cie::visible_wavelength_max - cie::visible_wavelength_min) / static_cast<float>(interval);
    uint n = ceil(factor);
    vector<float> samples(n);
    for (uint x = 0u; x < n; x++) { samples[x] = data[x * interval]; }
    return samples;
}

template<typename T, size_t N>
[[nodiscard]] vector<T> array2vector(const array<T, N> &arr) noexcept {
    vector<T> ret(N);
    for (int i = 0; i < N; ++i) {
        ret[i] = arr[i];
    }
    return ret;
}

}// namespace detail

SPD::SPD(vector<float> func, uint interval, RenderPipeline *rp)
    : _sample_interval(interval),
      _rp(rp) {
    _func.set_host(move(func));
}

SPD SPD::create_cie_x(RenderPipeline *rp) {
    return SPD(detail::array2vector(cie::X), spd_lut_interval, rp);
}

SPD SPD::create_cie_y(RenderPipeline *rp) {
    return SPD(detail::array2vector(cie::Y), spd_lut_interval, rp);
}

SPD SPD::create_cie_z(RenderPipeline *rp) {
    return SPD(detail::array2vector(cie::Z), spd_lut_interval, rp);
}

SPD SPD::create_cie_d65(RenderPipeline *rp) {
    return SPD(detail::array2vector(cie::D65), spd_lut_interval, rp);
}

}// namespace vision