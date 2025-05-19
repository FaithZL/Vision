//
// Created by Zero on 21/12/2022.
//

#include "base/color/spectrum.h"
#include "base/sampler.h"

namespace vision {

class SRGBSpectrum : public Spectrum {
public:
    SRGBSpectrum() = default;
    explicit SRGBSpectrum(const SpectrumDesc &desc)
        : Spectrum(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] SampledWavelengths sample_wavelength(TSampler &sampler) const noexcept override {
        SampledWavelengths swl{dimension()};
        auto lambdas = rgb_spectrum_peak_wavelengths;
        for (auto i = 0u; i < dimension(); i++) {
            swl.set_lambda(i, lambdas[i]);
            swl.set_pdf(i, 1.f);
        }
        return swl;
    }

    [[nodiscard]] float4 albedo_params(float4 rgb) const noexcept override {
        return make_float4(rgb.xyz(), ocarina::luminance(rgb.xyz()));
    }
    [[nodiscard]] float4 illumination_params(float4 rgb) const noexcept override {
        return make_float4(rgb.xyz(), ocarina::luminance(rgb.xyz()));
    }
    [[nodiscard]] float4 unbound_params(float4 rgb) const noexcept override {
        return make_float4(rgb.xyz(), ocarina::luminance(rgb.xyz()));
    }
    [[nodiscard]] Float cie_y(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return cie::linear_srgb_to_y(linear_srgb(sp, swl));
    }

    [[nodiscard]] Float luminance(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return ocarina::luminance(sp.vec3());
    }

    [[nodiscard]] Float3 cie_xyz(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return cie::linear_srgb_to_xyz(linear_srgb(sp, swl));
    }

    [[nodiscard]] Float3 linear_srgb(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return sp.vec3();
    }
    [[nodiscard]] ColorDecode decode_to_albedo(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        return {.sample = SampledSpectrum(rgb), .strength = ocarina::luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_illumination(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        return {.sample = SampledSpectrum(rgb), .strength = ocarina::luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_unbound_spectrum(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        return {.sample = SampledSpectrum(rgb), .strength = ocarina::luminance(rgb)};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, SRGBSpectrum)