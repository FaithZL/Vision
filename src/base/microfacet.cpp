//
// Created by Zero on 17/12/2022.
//

#include "microfacet.h"

namespace vision {

Float GGXMicrofacet::D_(Float3 wh) const noexcept {
    static Callable impl = [](Float3 wh, Float ax, Float ay) {
        return microfacet::D_<D>(wh, ax, ay, type);
    };
    return impl(wh, _alpha_x, _alpha_y);
}

Float3 GGXMicrofacet::sample_wh(const Float3 &wo, const Float2 &u) const noexcept {
    static Callable impl = [](Float3 wo, Float2 u, Float ax, Float ay) {
        return microfacet::sample_wh<D>(wo, u, ax, ay, type);
    };
    return impl(wo, u, _alpha_x, _alpha_y);
}
Float GGXMicrofacet::PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept {
    static Callable impl = [](Float3 wo, Float3 wh, Float ax, Float ay) {
        return microfacet::PDF_wh<D>(wo, wh, ax, ay, type);
    };
    return impl(wo, wh, _alpha_x, _alpha_y);
}

Float GGXMicrofacet::PDF_wi_reflection(Float pdf_wh, Float3 wo, Float3 wh) const noexcept {
    static Callable impl = [](Float pdf_wh, Float3 wo, Float3 wh) {
        return microfacet::PDF_wi_reflection<D>(pdf_wh, wo, wh);
    };
    return impl(pdf_wh, wo, wh);
}

Float GGXMicrofacet::PDF_wi_reflection(Float3 wo, Float3 wh) const noexcept {
    return PDF_wi_reflection(PDF_wh(wo, wh), wo, wh);
}

Float GGXMicrofacet::PDF_wi_transmission(Float pdf_wh, Float3 wo, Float3 wh,
                                         Float3 wi, Float eta) const noexcept {
    static Callable impl = [](Float pdf_wh, Float3 wo, Float3 wh, Float3 wi, Float eta) {
        return microfacet::PDF_wi_transmission<D>(pdf_wh, wo, wh, wi, eta);
    };
    return impl(pdf_wh, wo, wh, wi, eta);
}

Float GGXMicrofacet::PDF_wi_transmission(Float3 wo, Float3 wh, Float3 wi, Float eta) const noexcept {
    return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, eta);
}

Float3 GGXMicrofacet::BRDF(Float3 wo, Float3 wh, Float3 wi, Float3 Fr) const noexcept {
    static Callable impl = [](Float3 wo, Float3 wh, Float3 wi, Float3 Fr, Float ax, Float ay) {
        return microfacet::BRDF<D>(wo, wh, wi, Fr, ax, ay, type);
    };
    return impl(wo, wh, wi, Fr, _alpha_x, _alpha_y);
}

Float3 GGXMicrofacet::BRDF(Float3 wo, Float3 wi, Float3 Fr) const noexcept {
    Float3 wh = normalize(wo + wi);
    return BRDF(wo, wh, wi, Fr);
}

Float3 GGXMicrofacet::BTDF(Float3 wo, Float3 wh, Float3 wi,
                           Float3 Ft, Float eta) const noexcept {
    static Callable impl = [](Float3 wo, Float3 wh, Float3 wi, Float3 Ft,
                              Float eta, Float ax, Float ay) {
        return microfacet::BTDF<D>(wo, wh, wi, Ft, eta, ax, ay, type);
    };
    return impl(wo, wh, wi, Ft, eta, _alpha_x, _alpha_y);
}

Float3 GGXMicrofacet::BTDF(Float3 wo, Float3 wi, Float3 Ft, Float eta) const noexcept {
    Float3 wh = normalize(wo + wi * eta);
    return BTDF(wo, wh, wi, Ft, eta);
}

}// namespace vision