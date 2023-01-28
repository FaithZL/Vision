//
// Created by Zero on 21/12/2022.
//

#include "srgb2spec.h"
#include "base/color/spectrum.h"
#include "base/color/spd.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

class RGBSigmoidPolynomial {
private:
    array<Float, 3> _c;

private:
    [[nodiscard]] static Float _s(Float x) noexcept {
        return select(isinf(x), cast<float>(x > 0.0f),
                      0.5f * fma(x, rsqrt(fma(x, x, 1.f)), 1.f));
    }

public:
    RGBSigmoidPolynomial() noexcept = default;
    RGBSigmoidPolynomial(Float c0, Float c1, Float c2) noexcept
        : _c{c0, c1, c2} {}
    explicit RGBSigmoidPolynomial(Float3 c) noexcept : _c{c[0], c[1], c[2]} {}
    [[nodiscard]] Float operator()(Float lambda) const noexcept {
        return _s(fma(lambda, fma(lambda, _c[0], _c[1]), _c[2]));// c0 * x * x + c1 * x + c2
    }
};

class RGBToSpectrumTable {
public:
    static constexpr auto res = 64u;
    using coefficient_table_type = const float[3][res][res][res][4];

private:
    const coefficient_table_type &_coefficients;
    RenderPipeline *_rp{};
    uint _base_index{InvalidUI32};
    RHITexture _coefficient0;
    RHITexture _coefficient1;
    RHITexture _coefficient2;

private:
    [[nodiscard]] inline static auto _inverse_smooth_step(auto x) noexcept {
        return 0.5f - sin(asin(1.0f - 2.0f * x) * (1.0f / 3.0f));
    }

public:
    explicit RGBToSpectrumTable(const coefficient_table_type &coefficients, RenderPipeline *rp) noexcept
        : _coefficients{coefficients}, _rp(rp) {}

    void init() noexcept {
        _coefficient0 = _rp->device().create_texture(make_uint3(res), PixelStorage::FLOAT4);
        _coefficient1 = _rp->device().create_texture(make_uint3(res), PixelStorage::FLOAT4);
        _coefficient2 = _rp->device().create_texture(make_uint3(res), PixelStorage::FLOAT4);
    }

    void prepare() noexcept {

        _base_index = _rp->register_texture(_coefficient0) - 1;
        _rp->register_texture(_coefficient1);
        _rp->register_texture(_coefficient2);
        _coefficient0.upload_immediately(&_coefficients[0]);
        _coefficient1.upload_immediately(&_coefficients[1]);
        _coefficient2.upload_immediately(&_coefficients[2]);
    }
};

class RGBAlbedoSpectrum {
private:
    RGBSigmoidPolynomial _rsp;

public:
    explicit RGBAlbedoSpectrum(RGBSigmoidPolynomial rsp) noexcept : _rsp{move(rsp)} {}
    [[nodiscard]] Float sample(const Float &lambda) const noexcept { return _rsp(lambda); }
};

class RGBAIlluminationSpectrum {
private:
    RGBSigmoidPolynomial _rsp;
    Float _scale;

public:
    explicit RGBAIlluminationSpectrum(RGBSigmoidPolynomial rsp) noexcept : _rsp{move(rsp)} {}
    [[nodiscard]] Float sample(const Float &lambda) const noexcept { return _rsp(lambda); }
};

class HeroWavelengthSpectrum : public Spectrum {
private:
    uint _dimension{};
    SPD _white_point;
    RGBToSpectrumTable _rgb_to_spectrum_table;

public:
    explicit HeroWavelengthSpectrum(const SpectrumDesc &desc)
        : Spectrum(desc), _dimension(desc.dimension),
          _rgb_to_spectrum_table(sRGBToSpectrumTable_Data, render_pipeline()),
          _white_point(SPD::create_cie_d65(render_pipeline())) {
        _rgb_to_spectrum_table.init();
    }

    void prepare() noexcept override {
        _white_point.prepare();
        _rgb_to_spectrum_table.prepare();
    }
    [[nodiscard]] uint dimension() const noexcept override { return _dimension; }
    [[nodiscard]] Float4 srgb(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return make_float4(sp[0], sp[1], sp[2], 1.f);
    }
    [[nodiscard]] SampledWavelengths sample_wavelength(Sampler *sampler) const noexcept override {
        SampledWavelengths swl{3u};
        auto lambdas = rgb_spectrum_peak_wavelengths;
        for (auto i = 0u; i < 3u; i++) {
            swl.set_lambda(i, lambdas[i]);
            swl.set_pdf(i, 1.f);
        }
        return swl;
    }
    [[nodiscard]] ColorDecode decode_to_albedo(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        // todo
        return {.sample = SampledSpectrum(rgb), .strength = luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_illumination(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        // todo
        return {.sample = SampledSpectrum(rgb), .strength = luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_unbound_spectrum(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        // todo
        return {.sample = SampledSpectrum(rgb), .strength = luminance(rgb)};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::HeroWavelengthSpectrum)