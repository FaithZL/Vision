//
// Created by Zero on 28/10/2022.
//

#include "material.h"

namespace vision {

Int BSDF::match_num(Uchar bxdf_flag) const noexcept {
    Int ret = 0;
    for_each([&](const BxDF *bxdf) {
        ret += select(bxdf->match_flag(bxdf_flag), 1, 0);
    });
    return ret;
}

Uchar BSDF::flag() const noexcept {
    Uchar ret{0};
    for_each([&](const BxDF *bxdf) {
        ret |= bxdf->flag();
    });
    return ret;
}

Uchar BSDF::combine_flag(Float3 wo, Float3 wi, Uchar flag) noexcept {
    Bool reflect = same_hemisphere(wo, wi);
    uchar non_reflect = ~BxDF::Flag::Reflection;
    uchar non_trans = ~BxDF::Flag::Transmission;
    return select(reflect, flag & non_trans, flag & non_reflect);
}

Float BSDF::PDF_(Float3 wo, Float3 wi, Uchar flag) const noexcept {
    flag = combine_flag(wo, wi, flag);



    return ocarina::Float();
}

Float3 BSDF::eval_(Float3 wo, Float3 wi, Uchar flag) const noexcept {
    return ocarina::Float3();
}

BSDFSample BSDF::sample_(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept {
    return BSDFSample();
}

Float BSDF::PDF(ocarina::Float3 world_wo, ocarina::Float3 world_wi, ocarina::Uchar flag) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    Float3 wi = shading_frame.to_local(world_wi);
    return PDF_(wo, wi, flag);
}

Float3 BSDF::eval(Float3 world_wo, Float3 world_wi, Uchar flag) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    Float3 wi = shading_frame.to_local(world_wi);
    return eval_(wo, wi, flag);
}

BSDFSample BSDF::sample(Float3 world_wo, Float uc, Float2 u, Uchar flag) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    BSDFSample ret = sample_(wo, uc, u, flag);
    ret.wi = shading_frame.to_local(ret.wi);
    ret.val *= abs_dot(shading_frame.z, ret.wi);
    return ret;
}

}// namespace vision