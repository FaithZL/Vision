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
    Float ret{0.f};
    Int match_count{0};
    for_each([&](const BxDF *bxdf) {
        $if (bxdf->match_flag(flag)) {
            match_count += 1;
            ret += bxdf->safe_PDF(wo, wi);
        };
    });
    return select(match_count > 0, ret / cast<float>(match_count), 0);
}

Float3 BSDF::eval_(Float3 wo, Float3 wi, Uchar flag) const noexcept {
    Float3 ret{make_float3(0.f)};
    $if(wo.z != 0) {
        for_each([&](const BxDF *bxdf) {
            $if(bxdf->match_flag(flag)) {
                ret += bxdf->safe_eval(wo, wi);
            };
        });
    };
    return ret;
}

BSDFSample BSDF::sample_(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept {
    BSDFSample ret;
    Int num = match_num(flag);
    $if(num > 0) {
        Int comp = min(cast<int>(floor(uc * num)), num - 1);
        Int count = 0;
        for_each([&](const BxDF *bxdf) {
            $if(bxdf->match_flag(flag)) {
                $if(count == comp) {
                    ret = bxdf->sample(wo, u);
                };
                count += 1;
            };
        });
    };
    return ret;
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