//
// Created by Zero on 17/12/2022.
//

#include "microfacet.h"

namespace vision {

inline namespace microfacet {
template<EPort p>
[[nodiscard]] oc_float<p> D_(const oc_float3<p> &wh, oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type) {
    // When theta is close to 90, tan theta is infinity
    oc_float<p> tan_theta_2 = geometry::tan_theta_2(wh);
    oc_float<p> cos_theta_4 = sqr(geometry::cos_theta_2(wh));
    switch (type) {
        case Disney:
        case GGX: {
            oc_float<p> e = tan_theta_2 * (sqr(geometry::cos_phi(wh) / alpha_x) + sqr(geometry::sin_phi(wh) / alpha_y));
            oc_float<p> ret = 1.f / (Pi * alpha_x * alpha_y * cos_theta_4 * sqr(1 + e));
            return select(cos_theta_4 < 1e-16f || ocarina::isinf(tan_theta_2), 0.f, ret);
        }
        case Beckmann: {
            oc_float<p> ret = exp(-tan_theta_2 * (geometry::cos_phi_2(wh) / sqr(alpha_x) +
                                                  geometry::sin_phi_2(wh) / sqr(alpha_y))) /
                              (Pi * alpha_x * alpha_y * cos_theta_4);
            return select(cos_theta_4 < 1e-16f || ocarina::isinf(tan_theta_2), 0.f, ret);
        }
        default:
            break;
    }
    return 0;
}
template oc_float<D> D_<D>(const oc_float3<D> &wh, oc_float<D> alpha_x, oc_float<D> alpha_y, MicrofacetType type);

template<EPort p>
[[nodiscard]] oc_float<p> lambda(const oc_float3<p> &w, const oc_float<p> &alpha_x,
                                 const oc_float<p> &alpha_y, MicrofacetType type) {
    switch (type) {
        case Disney:
        case GGX: {
            oc_float<p> abs_tan_theta = abs(geometry::tan_theta(w));
            oc_float<p> cos_theta2 = geometry::cos_theta_2(w);
            oc_float<p> sin_theta2 = geometry::sin_theta_2(w);
            oc_float<p> alpha = sqrt(cos_theta2 * sqr(alpha_x) +
                                     sin_theta2 * sqr(alpha_y));
            oc_float<p> ret = (-1 + sqrt(1.f + sqr(alpha * abs_tan_theta))) / 2;
            return select(ocarina::isinf(abs_tan_theta), 0.f, ret);
        }
        case Beckmann: {
            oc_float<p> abs_tan_theta = abs(geometry::tan_theta(w));

            oc_float<p> cos_theta2 = geometry::cos_theta_2(w);
            oc_float<p> sin_theta2 = geometry::sin_theta_2(w);

            oc_float<p> alpha = sqrt(cos_theta2 * sqr(alpha_x) +
                                     sin_theta2 * sqr(alpha_y));
            oc_float<p> a = 1.f / (alpha * abs_tan_theta);

            oc_float<p> ret = (1 - 1.259f * a + 0.396f * sqr(a)) / (3.535f * a + 2.181f * sqr(a));
            return select(a >= 1.6f || ocarina::isinf(abs_tan_theta), 0.f, ret);
        }
        default:
            break;
    }
    return 0;
}
template oc_float<D> lambda<D>(const oc_float3<D> &w, const oc_float<D> &alpha_x,
                               const oc_float<D> &alpha_y, MicrofacetType type);

template<EPort p>
[[nodiscard]] oc_float3<p> sample_wh(const oc_float3<p> &wo, const oc_float2<p> &u,
                                     oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type) {
    switch (type) {
        case Disney:
        case GGX: {
            oc_float<p> cos_theta = 0, phi = _2Pi * u[1];
            phi = atan(alpha_y / alpha_x * tan(_2Pi * u[1] + PiOver2));
            phi = select(u[1] > .5f, phi + Pi, phi);
            oc_float<p> sin_phi = sin(phi), cos_phi = cos(phi);
            oc_float<p> alpha2 = 1.f / (sqr(cos_phi / alpha_x) + sqr(sin_phi / alpha_y));
            oc_float<p> tan_theta_2 = alpha2 * u[0] / (1 - u[0]);
            cos_theta = 1 / sqrt(1 + tan_theta_2);
            oc_float<p> sin_theta = safe_sqrt(1 - sqr(cos_theta));
            oc_float3<p> wh = spherical_direction<p>(sin_theta, cos_theta, phi);
            wh = select(same_hemisphere(wo, wh), wh, -wh);
            //            CHECK_UNIT_VEC(wh)
            return wh;
        }
        case Beckmann: {
            oc_float<p> tan_theta_2, phi;
            oc_float<p> log_sample = log(1 - u[0]);
            phi = atan(alpha_y / alpha_x *
                       tan(_2Pi * u[1] + PiOver2));
            phi = select(u[1] > .5f, phi + Pi, phi);
            oc_float<p> sin_phi = sin(phi), cos_phi = cos(phi);
            tan_theta_2 = -log_sample / (sqr(cos_phi / alpha_x) + sqr(sin_phi / alpha_y));
            oc_float<p> cos_theta = 1 / sqrt(1 + tan_theta_2);
            oc_float<p> sin_theta = safe_sqrt(1 - sqr(cos_theta));
            oc_float3<p> wh = spherical_direction<p>(sin_theta, cos_theta, phi);
            wh = select(same_hemisphere(wo, wh), wh, -wh);
            //            CHECK_UNIT_VEC(wh)
            return wh;
        }
        default:
            break;
    }
    return {};
}
template oc_float3<D> sample_wh<D>(const oc_float3<D> &wo, const oc_float2<D> &u,
                                   oc_float<D> alpha_x, oc_float<D> alpha_y, MicrofacetType type);

template<EPort p>
[[nodiscard]] oc_float<p> BTDF_div_ft(const oc_float3<p> &wo, const oc_float3<p> &wh, const oc_float3<p> &wi,
                                      const oc_float<p> &eta, const oc_float<p> &alpha_x,
                                      const oc_float<p> &alpha_y, MicrofacetType type) {
    oc_float<p> cos_theta_i = cos_theta(wi);
    oc_float<p> cos_theta_o = cos_theta(wo);
    oc_float<p> numerator = D_<p>(wh, alpha_x, alpha_y, type) * G_<p>(wo, wi, alpha_x, alpha_y, type) *
                            abs(dot(wi, wh) * dot(wo, wh));
    oc_float<p> denom = sqr(dot(wi, wh) * eta + dot(wo, wh)) * abs(cos_theta_i * cos_theta_o);
    oc_float<p> ft = numerator / denom;
    oc_float<p> factor = rcp(sqr(eta));
    return ft * factor;
}

template oc_float<D> BTDF_div_ft<D>(const oc_float3<D> &wo, const oc_float3<D> &wh, const oc_float3<D> &wi,
                                    const oc_float<D> &eta, const oc_float<D> &alpha_x,
                                    const oc_float<D> &alpha_y, MicrofacetType type);

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