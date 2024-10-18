//
// Created by Zero on 17/12/2022.
//

#include "microfacet.h"

namespace vision {

namespace microfacet {
[[nodiscard]] Float BTDF_div_ft(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                const Float &eta, const Float &alpha_x,
                                const Float &alpha_y, MicrofacetType type) {
    Float cos_theta_i = cos_theta(wi);
    Float cos_theta_o = cos_theta(wo);
    Float numerator = D_<D>(wh, alpha_x, alpha_y, type) * G_<D>(wo, wi, alpha_x, alpha_y, type) *
                      abs(dot(wi, wh) * dot(wo, wh));
    Float denom = sqr(dot(wi, wh) * eta + dot(wo, wh)) * abs(cos_theta_i * cos_theta_o);
    Float ft = numerator / denom;
    Float factor = rcp(sqr(eta));
    return ft * factor;
}
}// namespace microfacet

Float GGXMicrofacet::D_(Float3 wh) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wh, Float ax, Float ay) {
        return microfacet::D_<D>(wh, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::D");
    return impl(wh, alpha_x_, alpha_y_);
}

Float3 GGXMicrofacet::sample_wh(const Float3 &wo, const Float2 &u) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float2 u, Float ax, Float ay) {
        return microfacet::sample_wh<D>(wo, u, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::sample_wh");
    return impl(wo, u, alpha_x_, alpha_y_);
}
Float GGXMicrofacet::PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float3 wh, Float ax, Float ay) {
        return microfacet::PDF_wh<D>(wo, wh, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::PDF_wh");
    return impl(wo, wh, alpha_x_, alpha_y_);
}

Float GGXMicrofacet::PDF_wi_reflection(Float pdf_wh, Float3 wo, Float3 wh) const noexcept {
    static CALLABLE_TYPE impl = [](Float pdf_wh, Float3 wo, Float3 wh) {
        return microfacet::PDF_wi_reflection<D>(pdf_wh, wo, wh);
    };
    impl.function()->set_description("GGXMicrofacet::PDF_wi_reflection");
    return impl(pdf_wh, wo, wh);
}

Float GGXMicrofacet::PDF_wi_reflection(Float3 wo, Float3 wh) const noexcept {
    return PDF_wi_reflection(PDF_wh(wo, wh), wo, wh);
}

Float GGXMicrofacet::PDF_wi_transmission(Float pdf_wh, Float3 wo, Float3 wh,
                                         Float3 wi, Float eta) const noexcept {
    static CALLABLE_TYPE impl = [](Float pdf_wh, Float3 wo, Float3 wh, Float3 wi, Float eta) {
        return microfacet::PDF_wi_transmission<D>(pdf_wh, wo, wh, wi, eta);
    };
    impl.function()->set_description("GGXMicrofacet::PDF_wi_transmission");
    return impl(pdf_wh, wo, wh, wi, eta);
}

Float GGXMicrofacet::PDF_wi_transmission(Float3 wo, Float3 wh, Float3 wi, Float eta) const noexcept {
    return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, eta);
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BRDF(Float3 wo, Float3 wh, Float3 wi, const TSpectrum &Fr) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float3 wh, Float3 wi, Float ax, Float ay) {
        return microfacet::BRDF_div_fr<D>(wo, wh, wi, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::BRDF_div_fr");
    return impl(wo, wh, wi, alpha_x_, alpha_y_) * Fr;
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BRDF(Float3 wo, Float3 wi, const TSpectrum &Fr) const noexcept {
    Float3 wh = normalize(wo + wi);
    return BRDF(wo, wh, wi, Fr);
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BTDF(Float3 wo, Float3 wh, Float3 wi,
                                             const TSpectrum &Ft, Float eta) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float3 wh, Float3 wi,
                                   Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::BTDF_div_ft");
    return impl(wo, wh, wi, eta, alpha_x_, alpha_y_) * Ft;
}

Float GGXMicrofacet::BTDF(Float3 wo, Float3 wh, Float3 wi, const Float &Ft, Float eta) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float3 wh, Float3 wi,
                                   Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::BTDF_div_ft");
    return impl(wo, wh, wi, eta, alpha_x_, alpha_y_) * Ft;
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BTDF(Float3 wo, Float3 wi, const TSpectrum &Ft, Float eta) const noexcept {
    Float3 wh = normalize(wo + wi * eta);
    return BTDF(wo, wh, wi, Ft, eta);
}

Float BeckmannMicrofacet::D_(Float3 wh) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wh, Float ax, Float ay) {
        return microfacet::D_<D>(wh, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::D");
    return impl(wh, alpha_x_, alpha_y_);
}

Float3 BeckmannMicrofacet::sample_wh(const Float3 &wo, const Float2 &u) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float2 u, Float ax, Float ay) {
        return microfacet::sample_wh<D>(wo, u, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::sample_wh");
    return impl(wo, u, alpha_x_, alpha_y_);
}
Float BeckmannMicrofacet::PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float3 wh, Float ax, Float ay) {
        return microfacet::PDF_wh<D>(wo, wh, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::PDF_wh");
    return impl(wo, wh, alpha_x_, alpha_y_);
}

Float BeckmannMicrofacet::PDF_wi_reflection(Float pdf_wh, Float3 wo, Float3 wh) const noexcept {
    static CALLABLE_TYPE impl = [](Float pdf_wh, Float3 wo, Float3 wh) {
        return microfacet::PDF_wi_reflection<D>(pdf_wh, wo, wh);
    };
    impl.function()->set_description("BeckmannMicrofacet::PDF_wi_reflection");
    return impl(pdf_wh, wo, wh);
}

Float BeckmannMicrofacet::PDF_wi_reflection(Float3 wo, Float3 wh) const noexcept {
    return PDF_wi_reflection(PDF_wh(wo, wh), wo, wh);
}

Float BeckmannMicrofacet::PDF_wi_transmission(Float pdf_wh, Float3 wo, Float3 wh,
                                              Float3 wi, Float eta) const noexcept {
    static CALLABLE_TYPE impl = [](Float pdf_wh, Float3 wo, Float3 wh, Float3 wi, Float eta) {
        return microfacet::PDF_wi_transmission<D>(pdf_wh, wo, wh, wi, eta);
    };
    impl.function()->set_description("BeckmannMicrofacet::PDF_wi_transmission");
    return impl(pdf_wh, wo, wh, wi, eta);
}

Float BeckmannMicrofacet::PDF_wi_transmission(Float3 wo, Float3 wh, Float3 wi, Float eta) const noexcept {
    return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, eta);
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BRDF(Float3 wo, Float3 wh, Float3 wi, const TSpectrum &Fr) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float3 wh, Float3 wi, Float ax, Float ay) {
        return microfacet::BRDF_div_fr<D>(wo, wh, wi, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::BRDF_div_fr");
    return impl(wo, wh, wi, alpha_x_, alpha_y_) * Fr;
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BRDF(Float3 wo, Float3 wi, const TSpectrum &Fr) const noexcept {
    Float3 wh = normalize(wo + wi);
    return BRDF(wo, wh, wi, Fr);
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BTDF(Float3 wo, Float3 wh, Float3 wi,
                                                       const TSpectrum &Ft, Float eta) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float3 wh, Float3 wi,
                                   Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::BTDF_div_ft");
    return impl(wo, wh, wi, eta, alpha_x_, alpha_y_) * Ft;
}

Float BeckmannMicrofacet::BTDF(Float3 wo, Float3 wh, Float3 wi, const Float &Ft, Float eta) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wo, Float3 wh, Float3 wi,
                                   Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::BTDF_div_ft");
    return impl(wo, wh, wi, eta, alpha_x_, alpha_y_) * Ft;
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BTDF(Float3 wo, Float3 wi, const TSpectrum &Ft, Float eta) const noexcept {
    Float3 wh = normalize(wo + wi * eta);
    return BTDF(wo, wh, wi, Ft, eta);
}

}// namespace vision