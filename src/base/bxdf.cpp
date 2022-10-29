//
// Created by Zero on 28/10/2022.
//

#include "bxdf.h"
#include "math/geometry.h"
#include "math/warp.h"
#include "core/constants.h"

namespace vision {

Float BxDF::PDF(Float3 wo, Float3 wi) const noexcept {
    return cosine_hemisphere_PDF(wi.z);
}

Float BxDF::safe_PDF(Float3 wo, Float3 wi) const noexcept {
    return select(same_hemisphere(wo, wi), PDF(wo, wi), 0.f);
}

Float3 BxDF::safe_eval(Float3 wo, Float3 wi) const noexcept {
    return select(same_hemisphere(wo, wi), eval(wo, wi), make_float3(0.f));
}

BSDFSample BxDF::sample(Float3 wo, Float2 u) const noexcept {
    BSDFSample ret;
    ret.wi = square_to_cosine_hemisphere(u);
    wo.z = select(wo.z < 0.f, -wo.z, wo.z);
    ret.val = eval(wo, ret.wi);
    ret.pdf = PDF(wo, ret.wi);
    return ret;
}

Float3 LambertReflection::eval(Float3 wo, Float3 wi) const noexcept {
    return Kr * InvPi;
}

}// namespace vision