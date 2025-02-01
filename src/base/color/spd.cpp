//
// Created by Zero on 26/12/2022.
//

#include "spd.h"
#include "cie.h"

namespace vision {

using namespace ocarina;

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

SPD::SPD(Pipeline *rp)
    : rp_(rp), func_(rp->bindless_array()) {}

SPD::SPD(vector<float> func, Pipeline *rp)
    : SPD(rp) {
    init(ocarina::move(func));
}

void SPD::init(vector<float> func) noexcept {
    sample_interval_ = static_cast<float>(cie::cie_sample_count) / func.size();
    func_.set_host(ocarina::move(func));
}

void SPD::prepare() noexcept {
    func_.reset_device_buffer_immediately(rp_->device());
    func_.register_self();
    func_.upload_immediately();
}

void SPD::upload() noexcept {
    func_.upload_immediately();
}

float SPD::eval(float lambda) const noexcept {
    using namespace cie;
    using namespace cie;
    float t = (ocarina::clamp(lambda, visible_wavelength_min, visible_wavelength_max) - visible_wavelength_min) / sample_interval_.hv();
    float sample_count = static_cast<uint>((visible_wavelength_max - visible_wavelength_min) / sample_interval_.hv()) + 1u;
    uint i = static_cast<uint>(min(t, static_cast<float>(sample_count - 2u)));
    float l = func_.host_buffer().at(i);
    float r = func_.host_buffer().at(i + 1);
    return ocarina::lerp(fract(t), l, r);
}

Float SPD::eval(const Float &lambda) const noexcept {
    using namespace cie;
    Float t = (clamp(lambda, visible_wavelength_min, visible_wavelength_max) - visible_wavelength_min) / sample_interval_.hv();
    uint sample_count = static_cast<uint>((visible_wavelength_max - visible_wavelength_min) / sample_interval_.hv()) + 1u;
    Uint i = cast<uint>(min(t, static_cast<float>(sample_count - 2u)));
    Float l = rp_->buffer_var<float>(*func_.index()).read(i);
    Float r = rp_->buffer_var<float>(*func_.index()).read(i + 1);
    return lerp(fract(t), l, r);
}

DynamicArray<float> SPD::eval(const SampledWavelengths &swl) const noexcept {
    DynamicArray<float> values{swl.dimension()};
    for (int i = 0; i < swl.dimension(); ++i) {
        values[i] = eval(swl.lambda(i));
    }
    return values;
}

SPD SPD::create_cie_x(Pipeline *rp) noexcept {
    return {detail::downsample_densely_sampled_spectrum(spd_lut_interval, cie::X.data()), rp};
}

SPD SPD::create_cie_y(Pipeline *rp) noexcept {
    return {detail::downsample_densely_sampled_spectrum(spd_lut_interval, cie::Y.data()), rp};
}

SPD SPD::create_cie_z(Pipeline *rp) noexcept {
    return {detail::downsample_densely_sampled_spectrum(spd_lut_interval, cie::Z.data()), rp};
}

SPD SPD::create_cie_d65(Pipeline *rp) noexcept {
    return {detail::downsample_densely_sampled_spectrum(spd_lut_interval, cie::D65.data()), rp};
}

float SPD::cie_y_integral() noexcept {
    static float integral = detail::densely_sampled_spectrum_integral(spd_lut_interval, cie::Y.data());
    return integral;
}

}// namespace vision