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
    uchar non_reflect = ~BxDFFlag::Reflection;
    uchar non_trans = ~BxDFFlag::Transmission;
    return select(reflect, flag & non_trans, flag & non_reflect);
}

BSDFSample BSDF::sample_local(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept {
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

BSDFSample BSDF::sample(Float3 world_wo, Float uc, Float2 u, Uchar flag) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    BSDFSample ret = sample_local(wo, uc, u, flag);
    ret.wi = shading_frame.to_world(ret.wi);
    ret.eval.f *= abs_dot(shading_frame.z, ret.wi);
    return ret;
}

Evaluation BSDF::evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept {
    Evaluation ret;
    flag = combine_flag(wo, wi, flag);

    Int match_count{0};
    for_each([&](const BxDF *bxdf) {
        $if (bxdf->match_flag(flag)) {
            match_count += 1;
            auto [val, pdf] = bxdf->safe_evaluate(wo, wi);
            ret.f += val;
            ret.pdf += pdf;
        };
    });
    ret.pdf = select(match_count > 0, ret.pdf / cast<float>(match_count), 0.f);

    return ret;
}

Evaluation BSDF::evaluate(Float3 world_wo, Float3 world_wi, Uchar flag) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    Float3 wi = shading_frame.to_local(world_wi);
    Evaluation ret = evaluate_local(wo, wi, flag);
    ret.f *= abs_dot(shading_frame.z, world_wi);
    return ret;
}

}// namespace vision