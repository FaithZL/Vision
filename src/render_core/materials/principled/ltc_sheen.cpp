//
// Created by ling.zhu on 2025/1/30.
//

#include "ltc_sheen.h"
#include "ltc_sheen_table.inl.h"
#include "base/mgr/pipeline.h"

namespace vision {

SheenLTCTable *SheenLTCTable::s_sheen_table = nullptr;

SheenLTCTable &SheenLTCTable::instance() {
    if (s_sheen_table == nullptr) {
        s_sheen_table = new SheenLTCTable();
        HotfixSystem::instance().register_static_var("SheenLTCTable", s_sheen_table);
    }
    return *s_sheen_table;
}

void SheenLTCTable::destroy_instance() {
    if (s_sheen_table) {
        delete s_sheen_table;
        s_sheen_table = nullptr;
    }
}

void SheenLTCTable::init() noexcept {
    if (approx_.handle()) {
        return;
    }
    Pipeline *ppl = Global::instance().pipeline();
    approx_ = ppl->device().create_texture(make_uint2(res),
                                           PixelStorage::FLOAT4,
                                           "SheenLTC approx_");
    volume_ = ppl->device().create_texture(make_uint2(res),
                                           PixelStorage::FLOAT4,
                                           "SheenLTC volume_");
    approx_.upload_immediately(addressof(SheenLTCTableApprox));
    volume_.upload_immediately(addressof(SheenLTCTableVolume));
}

Float4 SheenLTCTable::sample_approx(const Float &alpha, const Float &cos_theta) noexcept {
    return approx_.sample(4, make_float2(alpha, cos_theta)).as_vec4();
}

Float4 SheenLTCTable::sample_volume(const Float &alpha, const Float &cos_theta) noexcept {
    return volume_.sample(4, make_float2(alpha, cos_theta)).as_vec4();
}

Float3 SheenLTC::rotate(const Float3 &v, const Float3 &axis,
                        const Float &angle) noexcept {
    Float s = sin(angle);
    Float c = cos(angle);
    return v * c + axis * dot(v, axis) * (1.f - c) + s * cross(axis, v);
}

Float SheenLTC::eval_ltc(const Float3 &wi) const noexcept {
    /*
        The (inverse) transform matrix `M^{-1}` is given by:

                     [[a    0    b]
            M^{-1} =  [0    a    0]
                      [0    0    1]]

        with `a = ltcCoeffs[0]`, `b = ltcCoeffs[1]` fetched from the
        table. The transformed direction `wi_origin` is therefore:

                                       [[a * wi.x + b * wi.z]
            wi_origin = M^{-1} * wi =  [a * wi.y              ]
                                        [wi.z                 ]]

        which is subsequently normalized. The determinant of the matrix is

            |M^{-1}| = a * a

        which is used to compute the Jacobian determinant of the complete
        mapping including the normalization.

        See the original paper [Heitz et al. 2016] for details about the LTC
        itself.
    */
    Float a = c_.x;
    Float b = c_.y;
    Float3 wi_origin = make_float3(a * wi.x + b * wi.z,
                                   a * wi.y,
                                   wi.z);
    Float length = ocarina::length(wi_origin);
    wi_origin /= length;
    Float det = sqr(a);
    Float jacobian = det / (length * length * length);
    return cosine_hemisphere_PDF(cos_theta(wi_origin)) * jacobian;
}

ScatterEval SheenLTC::evaluate_local(const Float3 &wo, const Float3 &wi,
                                     MaterialEvalMode mode, const Uint &flag) const noexcept {
    ScatterEval ret{*swl_};
    Float cos_theta_o = cos_theta(wo);
    Float cos_theta_i = cos_theta(wi);

    Float phi_std = phi(wo);

    Float3 wi_std = rotate(wi, make_float3(0, 0, 1), -phi_std);

    Float ltc_value = eval_ltc(wi_std);
    ret.f = ltc_value * tint_ * c_.z / cos_theta_i;
    ret.pdfs = ltc_value;
    ret.f = select(cos_theta_i < 0 || cos_theta_o < 0, 0.f, ret.f);
    return ret;
}

SampledDirection SheenLTC::sample_wi(const Float3 &wo, const Uint &flag,
                                     TSampler &sampler) const noexcept {
    /*  The (inverse) transform matrix `M^{-1}` is given by:

                     [[a    0    b   ]
            M^{-1} =  [0    a    0   ]
                      [0    0    1   ]]

        with `a    = ltcCoeffs[0]`, `b    = ltcCoeffs[1]` fetched from the
        table. The non-inverted matrix `M` is therefore:

                [[1/a    0      -b/a   ]
            M =  [0      1/a     0     ]
                 [0      0       1     ]]

        and the transformed direction wi is:

                                  [[wi_origin.x/a    - wi_origin.z*b   /a   ]
            wi = M * wi_origin =  [wi_origin.y/a                            ]
                                   [wi_origin.z                             ]]

        which is subsequently normalized.

        See the original paper [Heitz et al. 2016] for details about the LTC
        itself.
    */
    Float3 wi_origin = square_to_cosine_hemisphere(sampler->next_2d());
    Float a = c_[0],
          b = c_[1];
    Float3 wi = make_float3(wi_origin.x / a - wi_origin.z * b / a,
                            wi_origin.y / a,
                            wi_origin.z);
    SampledDirection sd;
    sd.pdf = 1;
    sd.wi = wi;
    return sd;
}

Float3 SheenLTC::sample_ltc(const Float2 &u) const noexcept {
    /*  The (inverse) transform matrix `M^{-1}` is given by:

                     [[a    0    b   ]
            M^{-1} =  [0    a    0   ]
                      [0    0    1   ]]

        with `a    = ltcCoeffs[0]`, `b    = ltcCoeffs[1]` fetched from the
        table. The non-inverted matrix `M` is therefore:

                [[1/a    0      -b/a   ]
            M =  [0      1/a     0     ]
                 [0      0       1     ]]

        and the transformed direction wi is:

                                  [[wi_origin.x/a    - wi_origin.z*b   /a   ]
            wi = M * wi_origin =  [wi_origin.y/a                            ]
                                   [wi_origin.z                             ]]

        which is subsequently normalized.

        See the original paper [Heitz et al. 2016] for details about the LTC
        itself.
    */
    Float3 wi_origin = square_to_cosine_hemisphere(u);

    Float a = c_[0],
          b = c_[1];
    Float3 wi = make_float3(wi_origin.x / a - wi_origin.z * b / a,
                            wi_origin.y / a,
                            wi_origin.z);
    return normalize(wi);
}

BSDFSample SheenLTC::sample_local(const Float3 &wo, const Uint &flag,
                                  TSampler &sampler) const noexcept {
    Float cos_theta_o = cos_theta(wo);
    Float3 wi_std = sample_ltc(sampler->next_2d());

    Float phi_std = phi(wo);

    Float3 wi_origin = rotate(wi_std, make_float3(0, 0, 1), phi_std);
    BSDFSample ret{*swl_};
    ret.eval = evaluate_local(wo, wi_origin, MaterialEvalMode::All, flag);
    ret.wi = wi_origin;
    ret.eval.f = select(cos_theta_o < 0.f, 0.f, ret.eval.f);
    return ret;
}

}// namespace vision