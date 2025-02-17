//
// Created by Zero on 17/12/2022.
//

#include "microfacet.h"
#include "math/warp.h"

namespace vision {

inline namespace microfacet {
template<EPort p>
[[nodiscard]] oc_float<p> bsdf_D(const oc_float3<p> &wh, const oc_float<p> &alpha_x,
                                 const oc_float<p> &alpha_y, MicrofacetType type) {
    // When theta is close to 90, tan theta is infinity
    oc_float<p> tan_theta_2 = geometry::tan_theta_2(wh);
    oc_float<p> cos_theta_4 = ocarina::sqr(geometry::cos_theta_2(wh));
    switch (type) {
        case GGX: {
            oc_float3<p> H = wh / make_float3(alpha_x, alpha_y, 1.f);
            oc_float<p> alpha2 = alpha_x * alpha_y;
            return InvPi / (alpha2 * ocarina::sqr(length_squared(H)));
        }
        case Beckmann: {
            oc_float<p> ret = ocarina::exp(-tan_theta_2 * (geometry::cos_phi_2(wh) / ocarina::sqr(alpha_x) +
                                                           geometry::sin_phi_2(wh) / ocarina::sqr(alpha_y))) /
                              (Pi * alpha_x * alpha_y * cos_theta_4);
            return ocarina::select(cos_theta_4 < 1e-16f || ocarina::isinf(tan_theta_2), 0.f, ret);
        }
        default:
            break;
    }
    return 0;
}
template oc_float<D> bsdf_D<D>(const oc_float3<D> &wh, const oc_float<D> &alpha_x,
                               const oc_float<D> &alpha_y, MicrofacetType type);
template oc_float<H> bsdf_D<H>(const oc_float3<H> &wh, const oc_float<H> &alpha_x,
                               const oc_float<H> &alpha_y, MicrofacetType type);

template<EPort p>
[[nodiscard]] oc_float<p> bsdf_lambda(const oc_float3<p> &w, const oc_float<p> &alpha_x,
                                      const oc_float<p> &alpha_y, MicrofacetType type) {
    switch (type) {
        case GGX: {
            oc_float<p> sqr_alpha_tan_n = (ocarina::sqr(alpha_x * w.x) + ocarina::sqr(alpha_y * w.y)) / ocarina::sqr(w.z);
            oc_float<p> ret = 0.5f * (ocarina::sqrt(1.0f + sqr_alpha_tan_n) - 1.0f);
            return ocarina::select(w.z == 0, 0.f, ret);
        }
        case Beckmann: {
            oc_float<p> abs_tan_theta = ocarina::abs(geometry::tan_theta(w));

            oc_float<p> cos_theta2 = geometry::cos_theta_2(w);
            oc_float<p> sin_theta2 = geometry::sin_theta_2(w);

            oc_float<p> alpha = ocarina::sqrt(cos_theta2 * ocarina::sqr(alpha_x) +
                                              sin_theta2 * ocarina::sqr(alpha_y));
            oc_float<p> a = 1.f / (alpha * abs_tan_theta);

            oc_float<p> ret = (1 - 1.259f * a + 0.396f * ocarina::sqr(a)) / (3.535f * a + 2.181f * ocarina::sqr(a));
            return ocarina::select(a >= 1.6f || ocarina::isinf(abs_tan_theta), 0.f, ret);
        }
        default:
            break;
    }
    return 0;
}
template oc_float<D> bsdf_lambda<D>(const oc_float3<D> &w, const oc_float<D> &alpha_x,
                                    const oc_float<D> &alpha_y, MicrofacetType type);
template oc_float<H> bsdf_lambda<H>(const oc_float3<H> &w, const oc_float<H> &alpha_x,
                                    const oc_float<H> &alpha_y, MicrofacetType type);

template<EPort p>
[[nodiscard]] oc_float3<p> sample_GGX_VNDF(const oc_float3<p> &Ve, const oc_float2<p> &u,
                                           const oc_float<p> &alpha_x,
                                           const oc_float<p> &alpha_y) {
    /// transform ellipsoid to sphere
    oc_float3<p> Vh = normalize(make_float3(alpha_x * Ve.x, alpha_y * Ve.y, Ve.z));
    oc_float<p> lenSq = ocarina::length_squared(Vh.xy().decay());

    /// build new frame with T1, T2, Vh
    /// T1 = cross(Z, Vh) / length(cross(Z, Vh))
    /// T2 = cross(Vh, T1)
    float3 Z = make_float3(1, 0, 0);
    oc_float3<p> T1 = select(lenSq > 1e-7f, make_float3(-Vh.y, Vh.x, 0.0f) / ocarina::sqrt(lenSq), Z);
    oc_float3<p> T2 = select(lenSq > 1e-7f, cross(Vh, T1), make_float3(0.0f, 1.0f, 0.0f));
    oc_float2<p> t = square_to_disk<p>(u);

    /// remapping the point on disk to projection on plane consist from T1, T2
    t.y = ocarina::lerp(0.5f * (1.0f + Vh.z), ocarina::safe_sqrt(1.0f - ocarina::sqr(t.x)), t.y);
    oc_float3<p> Nh = t.x * T1 + t.y * T2 + ocarina::safe_sqrt(1.0f - length_squared(t)) * Vh;
    oc_float3<p> Ne = normalize(make_float3(alpha_x * Nh.x, alpha_y * Nh.y, max(0.0f, Nh.z)));
    return Ne;
}

template<EPort p>
[[nodiscard]] oc_float3<p> sample_wh(const oc_float3<p> &wo, const oc_float2<p> &u, const oc_float<p> &alpha_x,
                                     const oc_float<p> &alpha_y, bool sample_visible, MicrofacetType type) {
    switch (type) {
        case GGX: {
            if (sample_visible) {
                /// https://jcgt.org/published/0007/04/01/
                oc_bool<p> flip = wo.z < 0;
                oc_float3<p> new_wo = wo;
                new_wo = ocarina::select(flip, -wo, wo);
                oc_float3<p> wh = sample_GGX_VNDF<p>(new_wo, u, alpha_x, alpha_y);
//                wh.z = ocarina::select(flip, -wh.z, wh.z);
                wh = ocarina::select(flip, -wh, wh);
                return wh;
            } else {
                oc_float<p> cos_theta = 0, phi = _2Pi * u[1];
                phi = atan(alpha_y / alpha_x * tan(_2Pi * u[1] + PiOver2));
                phi = ocarina::select(u[1] > .5f, phi + Pi, phi);
                oc_float<p> sin_phi = sin(phi), cos_phi = cos(phi);
                oc_float<p> alpha2 = 1.f / (ocarina::sqr(cos_phi / alpha_x) + ocarina::sqr(sin_phi / alpha_y));
                oc_float<p> tan_theta_2 = alpha2 * u[0] / (1 - u[0]);
                cos_theta = 1 / ocarina::sqrt(1 + tan_theta_2);
                oc_float<p> sin_theta = ocarina::safe_sqrt(1 - ocarina::sqr(cos_theta));
                oc_float3<p> wh = spherical_direction<p>(sin_theta, cos_theta, phi);
                wh = select(same_hemisphere(wo, wh), wh, -wh);
                return wh;
            }
        }
        case Beckmann: {
            oc_float<p> tan_theta_2, phi;
            oc_float<p> log_sample = log(1 - u[0]);
            phi = atan(alpha_y / alpha_x *
                       tan(_2Pi * u[1] + PiOver2));
            phi = ocarina::select(u[1] > .5f, phi + Pi, phi);
            oc_float<p> sin_phi = sin(phi), cos_phi = cos(phi);
            tan_theta_2 = -log_sample / (ocarina::sqr(cos_phi / alpha_x) + ocarina::sqr(sin_phi / alpha_y));
            oc_float<p> cos_theta = 1 / ocarina::sqrt(1 + tan_theta_2);
            oc_float<p> sin_theta = ocarina::safe_sqrt(1 - ocarina::sqr(cos_theta));
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
template oc_float3<D> sample_wh<D>(const oc_float3<D> &wo, const oc_float2<D> &u, const oc_float<D> &alpha_x,
                                   const oc_float<D> &alpha_y, bool sample_visible, MicrofacetType type);
template oc_float3<H> sample_wh<H>(const oc_float3<H> &wo, const oc_float2<H> &u, const oc_float<H> &alpha_x,
                                   const oc_float<H> &alpha_y, bool sample_visible, MicrofacetType type);

template<EPort p>
[[nodiscard]] oc_float<p> PDF_wh(const oc_float3<p> &wo, const oc_float3<p> &wh,
                                 const oc_float<p> &alpha_x, const oc_float<p> &alpha_y,
                                 bool sample_visible, MicrofacetType type) {
    if (sample_visible) {
        return bsdf_D<p>(wh, alpha_x, alpha_y, type) * bsdf_G1<p>(wo, alpha_x, alpha_y, type) *
               abs_dot(wo, wh) / abs_cos_theta(wo);
    }
    return bsdf_D<p>(wh, alpha_x, alpha_y, type) * abs_cos_theta(wh);
}
template oc_float<D> PDF_wh<D>(const oc_float3<D> &wo, const oc_float3<D> &wh,
                               const oc_float<D> &alpha_x, const oc_float<D> &alpha_y,
                               bool sample_visible, MicrofacetType type);

template<EPort p>
[[nodiscard]] oc_float<p> BTDF_div_ft(const oc_float3<p> &wo, const oc_float3<p> &wh, const oc_float3<p> &wi,
                                      const oc_float<p> &eta, const oc_float<p> &alpha_x,
                                      const oc_float<p> &alpha_y, MicrofacetType type) {
    oc_float<p> cos_theta_i = cos_theta(wi);
    oc_float<p> cos_theta_o = cos_theta(wo);
    oc_float<p> numerator = bsdf_D<p>(wh, alpha_x, alpha_y, type) * bsdf_G<p>(wo, wi, alpha_x, alpha_y, type) *
                            abs(dot(wi, wh) * dot(wo, wh));
    oc_float<p> denom = sqr(dot(wi, wh) * eta + dot(wo, wh)) * abs(cos_theta_i * cos_theta_o);
    oc_float<p> ft = numerator / denom;
    oc_float<p> factor = rcp(sqr(eta));
    return ft * factor;
}
template oc_float<D> BTDF_div_ft<D>(const oc_float3<D> &wo, const oc_float3<D> &wh, const oc_float3<D> &wi,
                                    const oc_float<D> &eta, const oc_float<D> &alpha_x,
                                    const oc_float<D> &alpha_y, MicrofacetType type);

}// namespace microfacet

Float GGXMicrofacet::bsdf_D(Float3 wh) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wh, Float ax, Float ay) {
        return microfacet::bsdf_D<D>(wh, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::D");
    return impl(wh, alpha_x_, alpha_y_);
}

Float3 GGXMicrofacet::sample_wh(const Float3 &wo, const Float2 &u) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, Float2 u, Float ax, Float ay) {
        return microfacet::sample_wh<D>(wo, u, ax, ay, false, type);
    };
    impl.function()->set_description("GGXMicrofacet::sample_wh");
    static CALLABLE_TYPE sample_visible_impl = [](const Float3 &wo, Float2 u, Float ax, Float ay) {
        return microfacet::sample_wh<D>(wo, u, ax, ay, true, type);
    };
    impl.function()->set_description("GGXMicrofacet::sample_wh_visible");
    if (sample_visible_) {
        return sample_visible_impl(wo, u, alpha_x_, alpha_y_);
    }
    return impl(wo, u, alpha_x_, alpha_y_);
}
Float GGXMicrofacet::PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh,
                                   Float ax, Float ay) {
        return microfacet::PDF_wh<D>(wo, wh, ax, ay, false, type);
    };
    static CALLABLE_TYPE impl_visible = [](const Float3 &wo, const Float3 &wh,
                                                  Float ax, Float ay) {
        return microfacet::PDF_wh<D>(wo, wh, ax, ay, true, type);
    };

    impl.function()->set_description("GGXMicrofacet::PDF_wh");
    impl_visible.function()->set_description("GGXMicrofacet::PDF_wh_visible");
    if (sample_visible_) {
        return impl_visible(wo, wh, alpha_x_, alpha_y_);
    }
    return impl(wo, wh, alpha_x_, alpha_y_);
}

Float GGXMicrofacet::PDF_wi_reflection(const Float &pdf_wh, const Float3 &wo,
                                       const Float3 &wh) const noexcept {
    static CALLABLE_TYPE impl = [](const Float &pdf_wh, const Float3 &wo,
                                   const Float3 &wh) {
        return microfacet::PDF_wi_reflection<D>(pdf_wh, wo, wh);
    };
    impl.function()->set_description("GGXMicrofacet::PDF_wi_reflection");
    return impl(pdf_wh, wo, wh);
}

Float GGXMicrofacet::PDF_wi_reflection(const Float3 &wo, const Float3 &wh) const noexcept {
    return PDF_wi_reflection(PDF_wh(wo, wh), wo, wh);
}

Float GGXMicrofacet::PDF_wi_transmission(const Float &pdf_wh, const Float3 &wo, const Float3 &wh,
                                         const Float3 &wi, const Float &eta) const noexcept {
    static CALLABLE_TYPE impl = [](const Float &pdf_wh, const Float3 &wo,
                                   const Float3 &wh, const Float3 &wi, const Float &eta) {
        return microfacet::PDF_wi_transmission<D>(pdf_wh, wo, wh, wi, eta);
    };
    impl.function()->set_description("GGXMicrofacet::PDF_wi_transmission");
    return impl(pdf_wh, wo, wh, wi, eta);
}

Float GGXMicrofacet::PDF_wi_transmission(const Float3 &wo, const Float3 &wh,
                                         const Float3 &wi, const Float &eta) const noexcept {
    return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, eta);
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BRDF(const Float3 &wo, const Float3 &wh,
                                             const Float3 &wi, const TSpectrum &Fr) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh,
                                   const Float3 &wi, Float ax, Float ay) {
        return microfacet::BRDF_div_fr<D>(wo, wh, wi, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::BRDF_div_fr");
    return impl(wo, wh, wi, alpha_x_, alpha_y_) * Fr;
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BRDF(const Float3 &wo, const Float3 &wi,
                                             const TSpectrum &Fr) const noexcept {
    Float3 wh = normalize(wo + wi);
    return BRDF(wo, wh, wi, Fr);
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BTDF(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                             const TSpectrum &Ft, const Float &eta) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                   Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::BTDF_div_ft");
    return impl(wo, wh, wi, eta, alpha_x_, alpha_y_) * Ft;
}

Float GGXMicrofacet::BTDF(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                          const Float &Ft, const Float &eta) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                   Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    impl.function()->set_description("GGXMicrofacet::BTDF_div_ft");
    return impl(wo, wh, wi, eta, alpha_x_, alpha_y_) * Ft;
}

GGXMicrofacet::TSpectrum GGXMicrofacet::BTDF(const Float3 &wo, const Float3 &wi,
                                             const TSpectrum &Ft, const Float &eta) const noexcept {
    Float3 wh = normalize(wo + wi * eta);
    return BTDF(wo, wh, wi, Ft, eta);
}

Float BeckmannMicrofacet::bsdf_D(Float3 wh) const noexcept {
    static CALLABLE_TYPE impl = [](Float3 wh, Float ax, Float ay) {
        return microfacet::bsdf_D<D>(wh, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::D");
    return impl(wh, alpha_x_, alpha_y_);
}

Float3 BeckmannMicrofacet::sample_wh(const Float3 &wo, const Float2 &u) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, Float2 u, Float ax, Float ay) {
        return microfacet::sample_wh<D>(wo, u, ax, ay, false, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::sample_wh");
    return impl(wo, u, alpha_x_, alpha_y_);
}
Float BeckmannMicrofacet::PDF_wh(const Float3 &wo, const Float3 &wh) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh,
                                   Float ax, Float ay) {
        return microfacet::PDF_wh<D>(wo, wh, ax, ay, false, type);
    };
    static CALLABLE_TYPE impl_sample_visible = [](const Float3 &wo, const Float3 &wh,
                                                  Float ax, Float ay) {
        return microfacet::PDF_wh<D>(wo, wh, ax, ay, true, type);
    };

    impl.function()->set_description("BeckmannMicrofacet::PDF_wh");
    impl_sample_visible.function()->set_description("BeckmannMicrofacet::PDF_wh_sample_visible");
    if (sample_visible_) {
        return impl_sample_visible(wo, wh, alpha_x_, alpha_y_);
    }
    return impl(wo, wh, alpha_x_, alpha_y_);
}

Float BeckmannMicrofacet::PDF_wi_reflection(const Float &pdf_wh, const Float3 &wo,
                                            const Float3 &wh) const noexcept {
    static CALLABLE_TYPE impl = [](const Float &pdf_wh, const Float3 &wo, const Float3 &wh) {
        return microfacet::PDF_wi_reflection<D>(pdf_wh, wo, wh);
    };
    impl.function()->set_description("BeckmannMicrofacet::PDF_wi_reflection");
    return impl(pdf_wh, wo, wh);
}

Float BeckmannMicrofacet::PDF_wi_reflection(const Float3 &wo, const Float3 &wh) const noexcept {
    return PDF_wi_reflection(PDF_wh(wo, wh), wo, wh);
}

Float BeckmannMicrofacet::PDF_wi_transmission(const Float &pdf_wh, const Float3 &wo, const Float3 &wh,
                                              const Float3 &wi, const Float &eta) const noexcept {
    static CALLABLE_TYPE impl = [](const Float &pdf_wh, const Float3 &wo, const Float3 &wh,
                                   const Float3 &wi, const Float &eta) {
        return microfacet::PDF_wi_transmission<D>(pdf_wh, wo, wh, wi, eta);
    };
    impl.function()->set_description("BeckmannMicrofacet::PDF_wi_transmission");
    return impl(pdf_wh, wo, wh, wi, eta);
}

Float BeckmannMicrofacet::PDF_wi_transmission(const Float3 &wo, const Float3 &wh,
                                              const Float3 &wi, const Float &eta) const noexcept {
    return PDF_wi_transmission(PDF_wh(wo, wh), wo, wh, wi, eta);
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BRDF(const Float3 &wo, const Float3 &wh,
                                                       const Float3 &wi, const TSpectrum &Fr) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh,
                                   const Float3 &wi, Float ax, Float ay) {
        return microfacet::BRDF_div_fr<D>(wo, wh, wi, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::BRDF_div_fr");
    return impl(wo, wh, wi, alpha_x_, alpha_y_) * Fr;
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BRDF(const Float3 &wo, const Float3 &wi,
                                                       const TSpectrum &Fr) const noexcept {
    Float3 wh = normalize(wo + wi);
    return BRDF(wo, wh, wi, Fr);
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BTDF(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                       const TSpectrum &Ft, const Float &eta) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                   Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::BTDF_div_ft");
    return impl(wo, wh, wi, eta, alpha_x_, alpha_y_) * Ft;
}

Float BeckmannMicrofacet::BTDF(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                               const Float &Ft, const Float &eta) const noexcept {
    static CALLABLE_TYPE impl = [](const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                   Float eta, Float ax, Float ay) {
        return microfacet::BTDF_div_ft<D>(wo, wh, wi, eta, ax, ay, type);
    };
    impl.function()->set_description("BeckmannMicrofacet::BTDF_div_ft");
    return impl(wo, wh, wi, eta, alpha_x_, alpha_y_) * Ft;
}

BeckmannMicrofacet::TSpectrum BeckmannMicrofacet::BTDF(const Float3 &wo, const Float3 &wi,
                                                       const TSpectrum &Ft, const Float &eta) const noexcept {
    Float3 wh = normalize(wo + wi * eta);
    return BTDF(wo, wh, wi, Ft, eta);
}
}// namespace vision