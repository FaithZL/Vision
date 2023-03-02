//
// Created by Zero on 17/12/2022.
//

#include "microfacet.h"

namespace vision {

Float GGXMicrofacet::D_(Float3 wh) const noexcept {
    static CALLABLE impl = [](Float3 wh, Float ax, Float ay) {
        return microfacet::D_<D>(wh, ax, ay, type);
    };
    return impl(wh, _alpha_x, _alpha_y);
}

Float3 GGXMicrofacet::sample_wh(const Float3 &wo, const Float2 &u) const noexcept {
    static CALLABLE impl = [](Float3 wo, Float2 u, Float ax, Float ay) {
        return microfacet::sample_wh<D>(wo, u, ax, ay, type);
    };
    return impl(wo, u, _alpha_x, _alpha_y);
}
Float GGXMicrofacet::PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept {
    static CALLABLE impl = [](Float3 wo, Float3 wh, Float ax, Float ay) {
        return microfacet::PDF_wh<D>(wo, wh, ax, ay, type);
    };
    return impl(wo, wh, _alpha_x, _alpha_y);
}

Float GGXMicrofacet::PDF_wi_reflection(Float pdf_wh, Float3 wo, Float3 wh) const noexcept {
    static CALLABLE impl = [](Float pdf_wh, Float3 wo, Float3 wh) {
        return microfacet::PDF_wi_reflection<D>(pdf_wh, wo, wh);
    };
    return impl(pdf_wh, wo, wh);
}

Float GGXMicrofacet::PDF_wi_reflection(Float3 wo, Float3 wh) const noexcept {
    return PDF_wi_reflection(PDF_wh(wo, wh), wo, wh);
}

Float GGXMicrofacet::PDF_wi_transmission(Float pdf_wh, Float3 wo, Float3 wh,
                                         Float3 wi, Float eta) const noexcept {
    static CALLABLE impl = [](Float pdf_wh, Float3 wo, Float3 wh, Float3 wi, Float eta) {
        return microfacet::PDF_wi_transmission<D>(pdf_wh, wo, wh, wi, eta);
    };
    return impl(pdf_wh, wo, wh, wi, eta);
}

Float GGXMicrofacet::PDF_wi_transmission(Float3 wo, Float3 wh, Float3 wi, Float eta) const noexcept {
    return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, eta);
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BRDF(Float3 wo, Float3 wh, Float3 wi, const TSpectrum &Fr) const noexcept {
    static CALLABLE impl = [](Float3 wo, Float3 wh, Float3 wi, Float ax, Float ay) {
        return microfacet::BRDF_div_fr<D>(wo, wh, wi, ax, ay, type);
    };
    return impl(wo, wh, wi, _alpha_x, _alpha_y) * Fr;
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BRDF(Float3 wo, Float3 wi, const TSpectrum &Fr) const noexcept {
    Float3 wh = normalize(wo + wi);
    return BRDF(wo, wh, wi, Fr);
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BTDF(Float3 wo, Float3 wh, Float3 wi,
                                             const TSpectrum &Ft, Float eta) const noexcept {
    static CALLABLE impl = [](Float3 wo, Float3 wh, Float3 wi,
                              Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    return impl(wo, wh, wi, eta, _alpha_x, _alpha_y) * Ft;
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BTDF(Float3 wo, Float3 wi, const TSpectrum &Ft, Float eta) const noexcept {
    Float3 wh = normalize(wo + wi * eta);
    return BTDF(wo, wh, wi, Ft, eta);
}

Float BeckmannMicrofacet::D_(Float3 wh) const noexcept {
    static CALLABLE impl = [](Float3 wh, Float ax, Float ay) {
        return microfacet::D_<D>(wh, ax, ay, type);
    };
    return impl(wh, _alpha_x, _alpha_y);
}

Float3 BeckmannMicrofacet::sample_wh(const Float3 &wo, const Float2 &u) const noexcept {
    static CALLABLE impl = [](Float3 wo, Float2 u, Float ax, Float ay) {
        return microfacet::sample_wh<D>(wo, u, ax, ay, type);
    };
    return impl(wo, u, _alpha_x, _alpha_y);
}
Float BeckmannMicrofacet::PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept {
    static CALLABLE impl = [](Float3 wo, Float3 wh, Float ax, Float ay) {
        return microfacet::PDF_wh<D>(wo, wh, ax, ay, type);
    };
    return impl(wo, wh, _alpha_x, _alpha_y);
}

Float BeckmannMicrofacet::PDF_wi_reflection(Float pdf_wh, Float3 wo, Float3 wh) const noexcept {
    static CALLABLE impl = [](Float pdf_wh, Float3 wo, Float3 wh) {
        return microfacet::PDF_wi_reflection<D>(pdf_wh, wo, wh);
    };
    return impl(pdf_wh, wo, wh);
}

Float BeckmannMicrofacet::PDF_wi_reflection(Float3 wo, Float3 wh) const noexcept {
    return PDF_wi_reflection(PDF_wh(wo, wh), wo, wh);
}

Float BeckmannMicrofacet::PDF_wi_transmission(Float pdf_wh, Float3 wo, Float3 wh,
                                         Float3 wi, Float eta) const noexcept {
    static CALLABLE impl = [](Float pdf_wh, Float3 wo, Float3 wh, Float3 wi, Float eta) {
        return microfacet::PDF_wi_transmission<D>(pdf_wh, wo, wh, wi, eta);
    };
    return impl(pdf_wh, wo, wh, wi, eta);
}

Float BeckmannMicrofacet::PDF_wi_transmission(Float3 wo, Float3 wh, Float3 wi, Float eta) const noexcept {
    return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, eta);
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BRDF(Float3 wo, Float3 wh, Float3 wi, const TSpectrum &Fr) const noexcept {
    static CALLABLE impl = [](Float3 wo, Float3 wh, Float3 wi, Float ax, Float ay) {
        return microfacet::BRDF_div_fr<D>(wo, wh, wi, ax, ay, type);
    };
    return impl(wo, wh, wi, _alpha_x, _alpha_y) * Fr;
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BRDF(Float3 wo, Float3 wi, const TSpectrum &Fr) const noexcept {
    Float3 wh = normalize(wo + wi);
    return BRDF(wo, wh, wi, Fr);
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BTDF(Float3 wo, Float3 wh, Float3 wi,
                                                       const TSpectrum &Ft, Float eta) const noexcept {
    static CALLABLE impl = [](Float3 wo, Float3 wh, Float3 wi,
                              Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    return impl(wo, wh, wi, eta, _alpha_x, _alpha_y) * Ft;
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BTDF(Float3 wo, Float3 wi, const TSpectrum &Ft, Float eta) const noexcept {
    Float3 wh = normalize(wo + wi * eta);
    return BTDF(wo, wh, wi, Ft, eta);
}

}// namespace vision