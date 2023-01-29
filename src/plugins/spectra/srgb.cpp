//
// Created by Zero on 21/12/2022.
//

#include "base/color/spectrum.h"

namespace vision {

class SRGBSpectrum : public Spectrum {
public:
    explicit SRGBSpectrum(const SpectrumDesc &desc)
        : Spectrum(desc) {}

    [[nodiscard]] SampledWavelengths sample_wavelength(Sampler *sampler) const noexcept override {
        SampledWavelengths swl{3u};
        auto lambdas = rgb_spectrum_peak_wavelengths;
        for (auto i = 0u; i < 3u; i++) {
            swl.set_lambda(i, lambdas[i]);
            swl.set_pdf(i, 1.f);
        }
        return swl;
    }

    [[nodiscard]] Float cie_y(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return cie::srgb_to_y(srgb(sp, swl));
    }

    [[nodiscard]] Float3 cie_xyz(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return cie::srgb_to_xyz(srgb(sp, swl));
    }

    [[nodiscard]] Float3 srgb(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return sp.vec3();
    }
    [[nodiscard]] ColorDecode decode_to_albedo(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        return {.sample = SampledSpectrum(rgb), .strength = luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_illumination(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        return {.sample = SampledSpectrum(rgb), .strength = luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_unbound_spectrum(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        return {.sample = SampledSpectrum(rgb), .strength = luminance(rgb)};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SRGBSpectrum)