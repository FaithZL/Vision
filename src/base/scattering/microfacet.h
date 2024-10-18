//
// Created by Zero on 07/11/2022.
//

#pragma once

#include "math/basic_types.h"
#include "dsl/dsl.h"
#include "math/base.h"
#include "math/geometry.h"
#include "math/optics.h"
#include "base/color/spectrum.h"

namespace vision {
using namespace ocarina;
enum MicrofacetType : uint8_t {
    GGX,
    Disney,
    Beckmann,
};

inline namespace microfacet {

template<EPort p = D>
[[nodiscard]] oc_float<p> roughness_to_alpha(oc_float<p> roughness) {
    return sqr(roughness);
}

template<EPort p = D>
[[nodiscard]] oc_float2<p> roughness_to_alpha(oc_float2<p> roughness) {
    return sqr(roughness);
}

template<EPort p = D>
[[nodiscard]] oc_float<p> alpha_to_roughness(oc_float<p> alpha) {
    return sqrt(alpha);
}

template<EPort p = D>
[[nodiscard]] oc_float2<p> alpha_to_roughness(oc_float2<p> alpha) {
    return sqrt(alpha);
}

/**
 *  beckmann
 *
 *             e^[-(tan_theta_h)^2 ((cos_theta_h)^2/ax^2 + (sin_theta_h)^2/ay^2)]
 * D(wh) = -------------------------------------------------------------------------
 *                                PI ax ay (cos_theta_h)^4
 *
 *  GGX
 *                                                    1
 * D(wh) = ---------------------------------------------------------------------------------------------------
 *             PI ax ay (cos_theta_h)^4 [1 + (tan_theta_h)^2 ((cos_theta_h)^2/ax^2 + (sin_theta_h)^2/ay^2)]^2
 *
 * from http://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models.html
 *
 * @param wh
 * @return
 */
template<EPort p = D>
[[nodiscard]] oc_float<p> D_(const oc_float3<p> &wh, oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type = GGX);

/**
 * lambda(w) = A-(w) / (A+(w) - A-(w))
 * @param  w [description]
 * @return   [description]
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> lambda(const oc_float3<p> &w, const oc_float<p> &alpha_x,
                                 const oc_float<p> &alpha_y, MicrofacetType type = GGX);

/**
 * smith occlusion function
 * G1(w) = 1 / (lambda(w) + 1)
 * @param  w [description]
 * @return   [description]
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> G1(const oc_float3<p> &w, oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type = GGX) {
    oc_float<p> ret = 1 / (1 + lambda<p>(w, alpha_x, alpha_y, type));
    return ret;
}

/**
 * G(wo, wi) = 1 / (lambda(wo) + lambda(wi) + 1)
 * @return   [description]
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> G_(const oc_float3<p> &wo, const oc_float3<p> &wi, oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type = GGX) {
    oc_float<p> ret = 0.f;
    switch (type) {
        case Disney: {
            ret = G1<p>(wi, alpha_x, alpha_y, type) * G1<p>(wo, alpha_x, alpha_y, type);
            return ret;
        }
        case GGX:
        case Beckmann: {
            ret = 1 / (1 + lambda<p>(wo, alpha_x, alpha_y, type) + lambda<p>(wi, alpha_x, alpha_y, type));
            return ret;
        }
        default:
            break;
    }
    return ret;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> sample_wh(const oc_float3<p> &wo, const oc_float2<p> &u,
                                     oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type = GGX);

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> PDF_wh(const oc_float3<p> &wo, const oc_float3<p> &wh,
                                 const oc_float<p> &alpha_x, const oc_float<p> &alpha_y,
                                 MicrofacetType type = GGX) {
    return D_<p>(wh, alpha_x, alpha_y, type) * abs_cos_theta(wh);
}

/**
 * pwi(wi) = dwh / dwi * pwh(wh) = pwh(wh) / 4cos_theta_h
 * @param PDF_wh
 * @param wo
 * @param wh
 * @return
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> PDF_wi_reflection(const oc_float<p> &pdf_wh,
                                            const oc_float3<p> &wo,
                                            const oc_float3<p> &wh) {
    oc_float<p> ret = pdf_wh / (4 * abs_dot(wo, wh));
    return ret;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> PDF_wi_reflection(const oc_float3<p> &wo, const oc_float3<p> &wh, const oc_float<p> &alpha_x,
                                            const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    return PDF_wi_reflection<p>(PDF_wh<p>(wo, wh, alpha_x, alpha_y, type), wo, wh);
}

/**
 * dwh  dwi
 *                   eta_i^2 |wi dot wh|
 * dwh/dwi = -----------------------------------------
 *            [eta_o(wh dot wo) + eta_i(wi dot wh)]^2
 * @tparam T
 * @param PDF_wh
 * @param eta eta_i / eta_o
 * @return
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> PDF_wi_transmission(const oc_float<p> &PDF_wh, const oc_float3<p> &wo, const oc_float3<p> &wh,
                                              const oc_float3<p> &wi, const oc_float<p> &eta) {
    oc_float<p> denom = sqr(dot(wi, wh) * eta + dot(wo, wh));
    oc_float<p> dwh_dwi = abs_dot(wi, wh) / denom;
    oc_float<p> ret = PDF_wh * dwh_dwi;
    return ret;
}

[[nodiscard]] inline DynamicArray<float> PDF_wi_transmission(const Float &PDF_wh, const Float3 &wo, const Float3 &wh,
                                                             const Float3 &wi, const DynamicArray<float> &etas) {
    DynamicArray<float> denom = sqr(dot(wi, wh) * etas + dot(wo, wh));
    DynamicArray<float> dwh_dwi = abs_dot(wi, wh) / denom;
    DynamicArray<float> ret = PDF_wh * dwh_dwi;
    return ret;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> BRDF_div_fr(const oc_float3<p> &wo, const oc_float3<p> &wh, const oc_float3<p> &wi,
                                      const oc_float<p> &alpha_x, const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    oc_float<p> cos_theta_i = cos_theta(wi);
    oc_float<p> cos_theta_o = cos_theta(wo);
    oc_float<p> ret = D_<p>(wh, alpha_x, alpha_y, type) * G_<p>(wo, wi, alpha_x, alpha_y, type) / abs(4 * cos_theta_o * cos_theta_i);
    return ret;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> BRDF_div_fr(const oc_float3<p> &wo, const oc_float3<p> &wi,
                                       const oc_float<p> &alpha_x, const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    oc_float3<p> wh = normalize(wo + wi);
    return BRDF_div_fr<p>(wo, wh, wi, alpha_x, alpha_y, type);
}

/**
 *
 * @param eta : eta_i / eta_o
 * @param mode
 * @return
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> BTDF_div_ft(const oc_float3<p> &wo, const oc_float3<p> &wh, const oc_float3<p> &wi,
                                      const oc_float<p> &eta, const oc_float<p> &alpha_x,
                                      const oc_float<p> &alpha_y, MicrofacetType type = GGX);

[[nodiscard]] SampledSpectrum BTDF_div_ft(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                          const SampledSpectrum &eta, const Float &alpha_x,
                                          const Float &alpha_y, MicrofacetType type = GGX);

/**
 *
 * @param eta : eta_i / eta_o
 * @param mode
 * @return
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> BTDF_div_ft(const oc_float3<p> &wo, const oc_float3<p> &wi, const oc_float<p> &cos_theta_i,
                                      const oc_float<p> &cos_theta_o, const oc_float<p> &eta,
                                      const oc_float<p> &alpha_x, const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    oc_float3<p> wh = normalize(wo + wi * eta);
    return BTDF_div_ft<p>(wo, wh, wi, cos_theta_i, cos_theta_o, eta, alpha_x, alpha_y, type);
}

}// namespace microfacet

template<EPort p = EPort::D, typename TSpectrum = SampledSpectrum>
class Microfacet {
protected:
    oc_float<p> alpha_x_{};
    oc_float<p> alpha_y_{};
    MicrofacetType type_{GGX};

public:
    explicit Microfacet(const oc_float2<p> &alpha, MicrofacetType type = GGX)
        : alpha_x_(alpha.x), alpha_y_(alpha.y), type_(type) {}
    Microfacet(oc_float<p> ax, oc_float<p> ay, MicrofacetType type = GGX)
        : alpha_x_(std::move(ax)), alpha_y_(std::move(ay)), type_(type) {}
    [[nodiscard]] oc_float<p> max_alpha() const noexcept { return max(alpha_x_, alpha_y_); }
    [[nodiscard]] virtual oc_float<p> D_(oc_float3<p> wh) const noexcept { return microfacet::D_<p>(wh, alpha_x_, alpha_y_, type_); }
    [[nodiscard]] virtual oc_float3<p> sample_wh(const oc_float3<p> &wo, const oc_float2<p> &u) const noexcept {
        return microfacet::sample_wh<p>(wo, u, alpha_x_, alpha_y_, type_);
    }
    [[nodiscard]] virtual oc_float<p> PDF_wh(const oc_float3<p> &wo, const oc_float3<p> &wh) const noexcept {
        return microfacet::PDF_wh<p>(wo, wh, alpha_x_, alpha_y_, type_);
    }

    [[nodiscard]] virtual oc_float<p> PDF_wi_reflection(oc_float<p> pdf_wh, oc_float3<p> wo, oc_float3<p> wh) const noexcept {
        return microfacet::PDF_wi_reflection<p>(pdf_wh, wo, wh);
    }

    [[nodiscard]] virtual oc_float<p> PDF_wi_reflection(oc_float3<p> wo, oc_float3<p> wh) const noexcept {
        return PDF_wi_reflection(PDF_wh(wo, wh), wo, wh);
    }

    [[nodiscard]] virtual oc_float<p> PDF_wi_transmission(oc_float<p> pdf_wh, oc_float3<p> wo, oc_float3<p> wh,
                                                          oc_float3<p> wi, oc_float<p> eta) const noexcept {
        return microfacet::PDF_wi_transmission<p>(pdf_wh, wo, wh, wi, eta);
    }

    [[nodiscard]] virtual oc_float<p> PDF_wi_transmission(oc_float3<p> wo, oc_float3<p> wh, oc_float3<p> wi, oc_float<p> eta) const noexcept {
        return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, eta);
    }

    [[nodiscard]] virtual DynamicArray<float> PDF_wi_transmission(const Float &pdf_wh, const Float3 &wo, const Float3 &wh,
                                                                  const Float3 &wi, const DynamicArray<float> &etas) const noexcept {
        return microfacet::PDF_wi_transmission(pdf_wh, wo, wh, wi, etas);
    }

    [[nodiscard]] virtual DynamicArray<float> PDF_wi_transmission(const Float3 &wo, const Float3 &wh,
                                                                  const Float3 &wi, const DynamicArray<float> &etas) const noexcept {
        return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, etas);
    }

    [[nodiscard]] virtual TSpectrum BRDF(oc_float3<p> wo, oc_float3<p> wh, oc_float3<p> wi, const TSpectrum &Fr) const noexcept {
        return microfacet::BRDF_div_fr<p>(wo, wh, wi, alpha_x_, alpha_y_, type_) * Fr;
    }

    [[nodiscard]] virtual TSpectrum BRDF(oc_float3<p> wo, oc_float3<p> wi, const TSpectrum &Fr) const noexcept {
        oc_float3<p> wh = normalize(wo + wi);
        return this->BRDF(wo, wh, wi, Fr);
    }

    [[nodiscard]] virtual TSpectrum BTDF(oc_float3<p> wo, oc_float3<p> wh, oc_float3<p> wi,
                                         const TSpectrum &Ft, oc_float<p> eta) const noexcept {
        return microfacet::BTDF_div_ft<p>(wo, wh, wi, eta, alpha_x_, alpha_y_, type_) * Ft;
    }

    [[nodiscard]] virtual oc_float<p> BTDF(oc_float3<p> wo, oc_float3<p> wh, oc_float3<p> wi,
                                           const oc_float<p> &Ft, oc_float<p> eta) const noexcept {
        return microfacet::BTDF_div_ft<p>(wo, wh, wi, eta, alpha_x_, alpha_y_, type_) * Ft;
    }

    [[nodiscard]] virtual TSpectrum BTDF(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                         const TSpectrum &Ft, const TSpectrum &eta) const noexcept {
        return microfacet::BTDF_div_ft(wo, wh, wi, eta, alpha_x_, alpha_y_, type_) * Ft;
    }

    [[nodiscard]] virtual TSpectrum BTDF(oc_float3<p> wo, oc_float3<p> wi, const TSpectrum &Ft, oc_float<p> eta) const noexcept {
        oc_float3<p> wh = normalize(wo + wi * eta);
        return this->BTDF(wo, wh, wi, Ft, eta);
    }
};

class GGXMicrofacet : public Microfacet<D, SampledSpectrum> {
public:
    using TSpectrum = SampledSpectrum;

private:
    static constexpr MicrofacetType type = MicrofacetType::GGX;
    using Super = Microfacet<D>;

public:
    explicit GGXMicrofacet(const Float2 &alpha) : Super(alpha, type) {}
    GGXMicrofacet(Float ax, Float ay) : Super(std::move(ax), std::move(ay), type) {}
    [[nodiscard]] Float D_(Float3 wh) const noexcept override;
    [[nodiscard]] Float3 sample_wh(const Float3 &wo, const Float2 &u) const noexcept override;
    [[nodiscard]] Float PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept override;
    [[nodiscard]] Float PDF_wi_reflection(Float pdf_wh, Float3 wo, Float3 wh) const noexcept override;
    [[nodiscard]] Float PDF_wi_reflection(Float3 wo, Float3 wh) const noexcept override;
    [[nodiscard]] Float PDF_wi_transmission(Float pdf_wh, Float3 wo, Float3 wh, Float3 wi, Float eta) const noexcept override;
    [[nodiscard]] Float PDF_wi_transmission(Float3 wo, Float3 wh, Float3 wi, Float eta) const noexcept override;
    [[nodiscard]] TSpectrum BRDF(Float3 wo, Float3 wh, Float3 wi, const TSpectrum &Fr) const noexcept override;
    [[nodiscard]] TSpectrum BRDF(Float3 wo, Float3 wi, const TSpectrum &Fr) const noexcept override;
    [[nodiscard]] TSpectrum BTDF(Float3 wo, Float3 wh, Float3 wi, const TSpectrum &Ft, Float eta) const noexcept override;
    [[nodiscard]] TSpectrum BTDF(const Float3 &wo, const Float3 &wh, const Float3 &wi, const TSpectrum &Ft,
                                 const TSpectrum &eta) const noexcept override;
    [[nodiscard]] Float BTDF(Float3 wo, Float3 wh, Float3 wi, const Float &Ft,
                             Float eta) const noexcept override;
    [[nodiscard]] TSpectrum BTDF(Float3 wo, Float3 wi, const TSpectrum &Ft, Float eta) const noexcept override;
};

class BeckmannMicrofacet : public Microfacet<D> {
public:
    using TSpectrum = SampledSpectrum;

public:
    static constexpr MicrofacetType type = MicrofacetType::Beckmann;
    using Super = Microfacet<D>;

public:
    explicit BeckmannMicrofacet(const Float2 &alpha) : Super(alpha, type) {}
    BeckmannMicrofacet(Float ax, Float ay) : Super(std::move(ax), std::move(ay), type) {}
    [[nodiscard]] Float D_(Float3 wh) const noexcept override;
    [[nodiscard]] Float3 sample_wh(const Float3 &wo, const Float2 &u) const noexcept override;
    [[nodiscard]] Float PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept override;
    [[nodiscard]] Float PDF_wi_reflection(Float pdf_wh, Float3 wo, Float3 wh) const noexcept override;
    [[nodiscard]] Float PDF_wi_reflection(Float3 wo, Float3 wh) const noexcept override;
    [[nodiscard]] Float PDF_wi_transmission(Float pdf_wh, Float3 wo, Float3 wh, Float3 wi, Float eta) const noexcept override;
    [[nodiscard]] Float PDF_wi_transmission(Float3 wo, Float3 wh, Float3 wi, Float eta) const noexcept override;
    [[nodiscard]] TSpectrum BRDF(Float3 wo, Float3 wh, Float3 wi, const TSpectrum &Fr) const noexcept override;
    [[nodiscard]] TSpectrum BRDF(Float3 wo, Float3 wi, const TSpectrum &Fr) const noexcept override;
    [[nodiscard]] TSpectrum BTDF(Float3 wo, Float3 wh, Float3 wi, const TSpectrum &Ft, Float eta) const noexcept override;
    [[nodiscard]] TSpectrum BTDF(const Float3 &wo, const Float3 &wh, const Float3 &wi, const TSpectrum &Ft,
                                 const TSpectrum &eta) const noexcept override;
    [[nodiscard]] Float BTDF(Float3 wo, Float3 wh, Float3 wi, const Float &Ft,
                             Float eta) const noexcept override;
    [[nodiscard]] TSpectrum BTDF(Float3 wo, Float3 wi, const TSpectrum &Ft, Float eta) const noexcept override;
};

}// namespace vision