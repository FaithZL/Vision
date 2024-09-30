//
// Created by Zero on 21/12/2022.
//

#pragma once

#include "dsl/dsl.h"
#include "cie.h"
#include "base/node.h"

namespace vision {
using namespace ocarina;

static constexpr float3 rgb_spectrum_peak_wavelengths = make_float3(602.785f, 539.285f, 445.772f);

class MaterialEvaluator;

class SampledWavelengths {
private:
    DynamicArray<float> lambdas_;
    mutable DynamicArray<float> pdfs_;
    uint scatter_pdf_dim_{1u};

public:
    explicit SampledWavelengths(uint dim, uint d = 1u) noexcept
        : lambdas_{dim}, pdfs_{dim}, scatter_pdf_dim_{d} {}
    OC_MAKE_MEMBER_GETTER(scatter_pdf_dim, )
    [[nodiscard]] Float lambda(const Uint &i) const noexcept { return lambdas_[i]; }
    [[nodiscard]] Float pdf(const Uint &i) const noexcept { return pdfs_[i]; }
    void set_lambda(const Uint &i, const Float &lambda) noexcept { lambdas_[i] = lambda; }
    void set_pdf(const Uint &i, const Float &p) const noexcept { pdfs_[i] = p; }
    [[nodiscard]] uint dimension() const noexcept { return static_cast<uint>(lambdas_.size()); }
    [[nodiscard]] Uint valid_dimension() const noexcept;
    void invalidation_channel(const Uint &idx) const noexcept { set_pdf(idx, 0); }
    [[nodiscard]] Float3 lambda_vec3() const noexcept {
        return make_float3(lambdas_[0], lambdas_[1], lambdas_[2]);
    }
    [[nodiscard]] Float4 lambda_vec4() const noexcept {
        return make_float4(lambdas_[0], lambdas_[1], lambdas_[2], lambdas_[3]);
    }
    [[nodiscard]] Float3 pdf_vec3() const noexcept {
        return make_float3(pdfs_[0], pdfs_[1], pdfs_[2]);
    }
    [[nodiscard]] Float4 pdf_vec4() const noexcept {
        return make_float4(pdfs_[0], pdfs_[1], pdfs_[2], pdfs_[3]);
    }
    [[nodiscard]] Bool secondary_valid() const noexcept;
    void invalidation_secondary() const noexcept;
    void check_dispersive(const TSpectrum &spectrum, const MaterialEvaluator &bsdf) const noexcept;
};

class SampledSpectrum {
private:
    DynamicArray<float> values_;

public:
    explicit SampledSpectrum(const DynamicArray<float> &value) noexcept
        : values_(value) {}
    SampledSpectrum(uint n, const Float &value) noexcept
        : values_(n) {
        for (int i = 0; i < n; ++i) {
            values_[i] = value;
        }
    }
    explicit SampledSpectrum(uint n = 1u) noexcept : SampledSpectrum{n, 0.f} {}
    explicit SampledSpectrum(const Float3 &value) noexcept : values_(3) {
        for (int i = 0; i < 3; ++i) {
            values_[i] = value[i];
        }
    }
    explicit SampledSpectrum(const Float4 &value) noexcept : values_(4) {
        for (int i = 0; i < 4; ++i) {
            values_[i] = value[i];
        }
    }
    explicit SampledSpectrum(const Float &value) noexcept : SampledSpectrum{1u, value} {}
    explicit SampledSpectrum(float value) noexcept : SampledSpectrum{1u, value} {}
    [[nodiscard]] uint dimension() const noexcept {
        return static_cast<uint>(values_.size());
    }
    [[nodiscard]] DynamicArray<float> &values() noexcept { return values_; }
    [[nodiscard]] const DynamicArray<float> &values() const noexcept { return values_; }
    [[nodiscard]] Float &operator[](const Uint &i) noexcept {
        return dimension() == 1u ? values_[0u] : values_[i];
    }
    [[nodiscard]] Float operator[](const Uint &i) const noexcept {
        return dimension() == 1u ? values_[0u] : values_[i];
    }
    SampledSpectrum &operator=(const Float &value) noexcept {
        values_ = value;
        return *this;
    }
    SampledSpectrum &operator=(const DynamicArray<float> &value) noexcept {
        values_ = value;
        return *this;
    }
    [[nodiscard]] Float3 vec3() const noexcept {
        return values_.as_vec3();
    }
    [[nodiscard]] Float4 vec4() const noexcept {
        return values_.as_vec4();
    }
    template<typename F>
    [[nodiscard]] auto map(F &&f) const noexcept {
        SampledSpectrum s{dimension()};
        for (auto i = 0u; i < dimension(); i++) {
            if constexpr (std::invocable<F, Float>) {
                s[i] = f((*this)[i]);
            } else {
                s[i] = f(i, (*this)[i]);
            }
        }
        return s;
    }
    void sanitize() noexcept { values_.sanitize(); }
    [[nodiscard]] Float sum() const noexcept { return values().sum(); }
    [[nodiscard]] Float max() const noexcept { return values().max(); }
    [[nodiscard]] Float min() const noexcept { return values().min(); }
    [[nodiscard]] Float average() const noexcept {
        return sum() * static_cast<float>(1.0 / dimension());
    }
    template<typename F>
    [[nodiscard]] Bool any(F &&f) const noexcept { return values().any(OC_FORWARD(f)); }
    template<typename F>
    [[nodiscard]] Bool all(F &&f) const noexcept { return values().all(OC_FORWARD(f)); }
    [[nodiscard]] Bool is_zero() const noexcept {
        return all([](auto x) noexcept { return x == 0.f; });
    }
    template<typename F>
    [[nodiscard]] Bool none(F &&f) const noexcept { return !any(std::forward<F>(f)); }
    [[nodiscard]] SampledSpectrum operator+() const noexcept { return *this; }
    [[nodiscard]] SampledSpectrum operator-() const noexcept {
        return SampledSpectrum(-values());
    }

#define VS_MAKE_SPECTRUM_OPERATOR(op)                                                                                       \
    [[nodiscard]] SampledSpectrum operator op(const Float &rhs) const noexcept {                                            \
        return SampledSpectrum(values() op rhs);                                                                            \
    }                                                                                                                       \
    [[nodiscard]] SampledSpectrum operator op(const DynamicArray<float> &rhs) const noexcept {                              \
        return SampledSpectrum(values() op rhs);                                                                            \
    }                                                                                                                       \
    [[nodiscard]] SampledSpectrum operator op(const SampledSpectrum &rhs) const noexcept {                                  \
        OC_ASSERT(dimension() == 1 || rhs.dimension() == 1 || dimension() == rhs.dimension());                              \
        SampledSpectrum s(values() op rhs.values());                                                                        \
        return s;                                                                                                           \
    }                                                                                                                       \
    [[nodiscard]] friend SampledSpectrum operator op(const Float &lhs, const SampledSpectrum &rhs) noexcept {               \
        return SampledSpectrum(lhs op rhs.values());                                                                        \
    }                                                                                                                       \
    [[nodiscard]] friend SampledSpectrum operator op(const DynamicArray<float> &lhs, const SampledSpectrum &rhs) noexcept { \
        return SampledSpectrum(lhs op rhs.values());                                                                        \
    }                                                                                                                       \
    SampledSpectrum &operator op##=(const Float & rhs) noexcept {                                                           \
        values_ op## = rhs;                                                                                                 \
        return *this;                                                                                                       \
    }                                                                                                                       \
    SampledSpectrum &operator op##=(const DynamicArray<float> &rhs) noexcept {                                              \
        values_ op## = rhs;                                                                                                 \
        return *this;                                                                                                       \
    }                                                                                                                       \
    SampledSpectrum &operator op##=(const SampledSpectrum & rhs) noexcept {                                                 \
        OC_ASSERT(dimension() == 1 || rhs.dimension() == 1 || dimension() == rhs.dimension());                              \
        values_ op## = rhs.values();                                                                                        \
        return *this;                                                                                                       \
    }
    VS_MAKE_SPECTRUM_OPERATOR(+)
    VS_MAKE_SPECTRUM_OPERATOR(-)
    VS_MAKE_SPECTRUM_OPERATOR(*)
    VS_MAKE_SPECTRUM_OPERATOR(/)
#undef VS_MAKE_SPECTRUM_OPERATOR
};

template<typename A, typename B, typename T>
requires std::disjunction_v<
    std::is_same<std::remove_cvref_t<A>, SampledSpectrum>,
    std::is_same<std::remove_cvref_t<B>, SampledSpectrum>,
    std::is_same<std::remove_cvref_t<T>, SampledSpectrum>>
[[nodiscard]] auto lerp(T &&t, A &&a, B &&b) noexcept {
    return t * (b - a) + a;
}

[[nodiscard]] SampledSpectrum select(const SampledSpectrum &p, const SampledSpectrum &t, const SampledSpectrum &f) noexcept;
[[nodiscard]] SampledSpectrum select(const SampledSpectrum &p, const Float &t, const SampledSpectrum &f) noexcept;
[[nodiscard]] SampledSpectrum select(const SampledSpectrum &p, const SampledSpectrum &t, const Float &f) noexcept;
[[nodiscard]] SampledSpectrum select(const Bool &p, const SampledSpectrum &t, const SampledSpectrum &f) noexcept;
[[nodiscard]] SampledSpectrum select(const Bool &p, const Float &t, const SampledSpectrum &f) noexcept;
[[nodiscard]] SampledSpectrum select(const Bool &p, const SampledSpectrum &t, const Float &f) noexcept;

[[nodiscard]] SampledSpectrum zero_if_any_nan(const SampledSpectrum &t) noexcept;
[[nodiscard]] SampledSpectrum zero_if_any_nan_inf(const SampledSpectrum &t) noexcept;

#define VS_MAKE_SPECTRUM_MATH_FUNC(func_name)                                                        \
    template<typename... Args>                                                                       \
    [[nodiscard]] SampledSpectrum func_name(const SampledSpectrum &sp, Args &&...args) noexcept {    \
        return sp.map([&](Float s) noexcept { return ocarina::func_name(s, OC_FORWARD(args)...); }); \
    }
VS_MAKE_SPECTRUM_MATH_FUNC(clamp)
VS_MAKE_SPECTRUM_MATH_FUNC(exp)
VS_MAKE_SPECTRUM_MATH_FUNC(saturate)
VS_MAKE_SPECTRUM_MATH_FUNC(abs)
VS_MAKE_SPECTRUM_MATH_FUNC(rcp)
VS_MAKE_SPECTRUM_MATH_FUNC(sqrt)
VS_MAKE_SPECTRUM_MATH_FUNC(sqr)
VS_MAKE_SPECTRUM_MATH_FUNC(safe_sqrt)

#undef VS_MAKE_SPECTRUM_MATH_FUNC

struct ColorDecode {
    SampledSpectrum sample;
    Float strength;
    [[nodiscard]] static ColorDecode constant(uint dim, float value) noexcept {
        return ColorDecode{.sample = {dim, value}, .strength = value};
    }
    [[nodiscard]] static ColorDecode one(uint dim) noexcept { return constant(dim, 1.f); }
    [[nodiscard]] static ColorDecode zero(uint dim) noexcept { return constant(dim, 0.f); }
};

class Sampler;
template<typename T, typename Desc>
class TObject;
using TSampler = TObject<Sampler, SamplerDesc>;

class Spectrum : public Node {
public:
    using Desc = SpectrumDesc;

public:
    Spectrum() = default;
    explicit Spectrum(const SpectrumDesc &desc) : Node(desc) {}
    [[nodiscard]] SampledSpectrum zero() const noexcept { return SampledSpectrum{dimension(), 0.f}; }
    [[nodiscard]] SampledSpectrum one() const noexcept { return SampledSpectrum{dimension(), 1.f}; }
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] virtual SampledWavelengths sample_wavelength(TSampler &sampler) const noexcept = 0;
    [[nodiscard]] virtual uint dimension() const noexcept { return 3; }
    [[nodiscard]] virtual uint scatter_pdf_dim() const noexcept { return 1; }
    [[nodiscard]] virtual bool is_complete() const noexcept { return false; }
    [[nodiscard]] virtual optional<Bool> is_dispersive(const MaterialEvaluator *bsdf) const noexcept { return {}; }
    [[nodiscard]] virtual float4 albedo_params(float4 rgb) const noexcept = 0;
    [[nodiscard]] virtual float4 illumination_params(float4 rgb) const noexcept = 0;
    [[nodiscard]] virtual float4 unbound_params(float4 rgb) const noexcept = 0;
    [[nodiscard]] virtual Float3 linear_srgb(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual Float luminance(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual Float cie_y(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual Float3 cie_xyz(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual ColorDecode decode_to_albedo(Float3 rgb, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual ColorDecode decode_to_illumination(Float3 rgb, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual ColorDecode decode_to_unbound_spectrum(Float3 rgb, const SampledWavelengths &swl) const noexcept = 0;
};

using TSpectrum = TObject<Spectrum>;

}// namespace vision