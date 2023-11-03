//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"
#include "base/sampler.h"
#include "base/mgr/scene.h"
#include "descriptions/parameter_set.h"

namespace vision {
using namespace ocarina;

struct SpatialResamplingParam {
public:
    float dot_threshold{};
    float depth_threshold{};
    float sampling_radius{};
    uint iterate_num{};

public:
    SpatialResamplingParam() = default;
    explicit SpatialResamplingParam(const ParameterSet &ps)
        : dot_threshold(cosf(radians(ps["theta"].as_float(5)))),
          depth_threshold(ps["depth"].as_float(0.01f)),
          sampling_radius(ps["radius"].as_float(3.f)),
          iterate_num(ps["iterate_num"].as_uint(5)) {}
};

struct TemporalResamplingParam {
public:
    uint history_limit{};
    float sampling_radius{};
    float2 motion_vec_threshold{};
    float dot_threshold{};
    float depth_threshold{};

public:
    TemporalResamplingParam() = default;
    TemporalResamplingParam(const ParameterSet &ps, uint2 res)
        : history_limit(ps["history"].as_uint(10)),
          sampling_radius(ps["radius"].as_float(2.f)),
          motion_vec_threshold(ps["motion_vec"].as_float2(make_float2(0.15f)) * make_float2(res)),
          dot_threshold(cosf(radians(ps["theta"].as_float(5)))),
          depth_threshold(ps["depth"].as_float(0.1f)) {}
};

struct RSVSample {
    uint light_index{};
    uint prim_id{};
    float2 u;
    float p_hat;
    float pdf;
    array<float, 3> pos;
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
    void set_pos(float3 p) noexcept {
        pos[0] = p[0];
        pos[1] = p[1];
        pos[2] = p[2];
    }
};
}// namespace vision

// clang-format off
OC_STRUCT(vision::RSVSample, light_index, prim_id, u, p_hat, pdf, pos) {
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
    void set_pos(Float3 p) noexcept {
        pos[0] = p[0];
        pos[1] = p[1];
        pos[2] = p[2];
    }
};
// clang-format on

namespace vision {
using namespace ocarina;
struct SurfaceData {
    Hit hit{};
    float4 normal_t;
    uint mat_id{};
};
}// namespace vision

// clang-format off
OC_STRUCT(vision::SurfaceData, hit, normal_t, mat_id) {
    void set_normal(const Float3 &n) {
        normal_t = make_float4(n, normal_t.w);
    }
    [[nodiscard]] Float3 normal() const noexcept { return normal_t.xyz();}
    void set_t_max(const Float &t) { normal_t.w = t; }
    [[nodiscard]] Bool valid() const { return t_max() > 0.f; }
    [[nodiscard]] Float t_max() const noexcept { return normal_t.w;}
};
// clang-format on

namespace vision {
using OCSurfaceData = Var<SurfaceData>;
using OCRSVSample = Var<RSVSample>;
}// namespace vision

namespace vision {
using namespace ocarina;

struct Reservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    oc_float<p> M{};
    oc_float<p> W{};
    oc_uint<p> sample_num{};
    RSVSample sample{};

public:
    bool update(oc_float<p> u, oc_float<p> weight, RSVSample v) {
        weight_sum += weight;
        M += 1;
        sample_num += 1;
        bool ret = u < (weight / weight_sum);
        sample = ocarina::select(ret, v, sample);
        return ret;
    }
    void normalize() noexcept {
        weight_sum /= M;
        M = 1.f;
    }
    bool update(oc_float<p> u, RSVSample v) {
        oc_float<p> weight = ocarina::select(v.pdf == 0, 0.f, v.p_hat / v.pdf);
        return update(u, weight, v);
    }
    void update_W() noexcept {
        auto denominator = M * sample.p_hat;
        W = ocarina::select(denominator == 0.f, 0.f, weight_sum / denominator);
    }
    [[nodiscard]] auto compute_weight_sum() const noexcept {
        return sample.p_hat * W * M;
    }
    void reset_W() noexcept { W = 0.f; }
};

}// namespace vision

OC_STRUCT(vision::Reservoir, weight_sum, M, W, sample_num, sample) {
    static constexpr EPort p = D;
    void normalize() noexcept {
        weight_sum /= M;
        M = 1.f;
    }
    Bool update(oc_float<p> u, oc_float<p> weight, vision::OCRSVSample v) {
        weight_sum += weight;
        M += 1;
        sample_num += 1;
        Bool ret = u < (weight / weight_sum);
        sample = select(ret, v, sample);
        return ret;
    }
    Bool update(oc_float<p> u, vision::OCRSVSample v) {
        oc_float<p> weight = ocarina::select(v.pdf == 0, 0.f, v.p_hat / v.pdf);
        return update(u, weight, v);
    }
    void update_W() noexcept {
        Float denominator = M * sample.p_hat;
        W = ocarina::select(denominator == 0.f, 0.f, weight_sum / denominator);
    }
    void update_W_MIS(const Float &p_sum) noexcept {
        Float mis = sample.p_hat / p_sum;
        W = ocarina::select(sample.p_hat == 0.f, 0.f, mis * weight_sum / sample.p_hat);
    }
    [[nodiscard]] auto compute_weight_sum() const noexcept {
        return sample.p_hat * W * M;
    }
    void reset_W() noexcept { W = 0.f; }
};

namespace vision {

[[nodiscard]] inline Float compute_p_hat(const Scene &scene, const Interaction &it,
                                         SampledWavelengths &swl,
                                         const OCRSVSample &sample,
                                         LightSample *output_ls = nullptr) noexcept {
    LightSampler *light_sampler = scene.light_sampler();
    Spectrum &spectrum = *scene.spectrum();
    SampledLight sampled_light;
    sampled_light.light_index = sample.light_index;
    sampled_light.PMF = light_sampler->PMF(it, sample.light_index);
    LightSample ls = light_sampler->sample(sampled_light, it, sample.u, swl);
    Float3 wi = normalize(ls.p_light - it.pos);
    SampledSpectrum f{swl.dimension()};
    ScatterEval eval{swl.dimension()};
    scene.materials().dispatch(it.material_id(), [&](const Material *material) {
        BSDF bsdf = material->compute_BSDF(it, swl);
        if (auto dispersive = spectrum.is_dispersive(&bsdf)) {
            $if(*dispersive) {
                swl.invalidation_secondary();
            };
        }
        eval = bsdf.evaluate(it.wo, wi);
    });
    f = eval.f * ls.eval.L;
    if (output_ls) {
        *output_ls = ls;
    }
    Float p_hat = f.average();
    return p_hat;
}

using namespace ocarina;
using OCReservoir = Var<Reservoir>;
[[nodiscard]] inline OCReservoir combine_reservoir(const OCReservoir &r0,
                                                   const OCReservoir &r1,
                                                   const Float &u) noexcept {
    OCReservoir ret;
    ret = r0;
    ret->update(u, r1->compute_weight_sum(), r1.sample);
    ret.M = r0.M + r1.M;
    ret->update_W();
    return ret;
}

[[nodiscard]] inline OCReservoir combine_reservoirs(OCReservoir cur_rsv,
                                                    const Container<uint> &rsv_idx,
                                                    const Buffer<Reservoir> &reservoirs,
                                                    Sampler *sampler) noexcept {
    rsv_idx.for_each([&](const Uint &idx) {
        OCReservoir rsv = reservoirs.read(idx);
        cur_rsv->update(sampler->next_1d(), rsv->compute_weight_sum(), rsv.sample);
        cur_rsv.M += rsv.M;
    });
    cur_rsv->update_W();
    return cur_rsv;
}

[[nodiscard]] inline OCReservoir combine_reservoir_MIS(const OCReservoir &r0,
                                                       const OCReservoir &r1,
                                                       const Float &u,
                                                       Float *p_sum) noexcept {
    OCReservoir ret;
    ret = r0;
    ret->update(u, r1->compute_weight_sum(), r1.sample);
    ret.M = r0.M + r1.M;
    *p_sum += r1.sample.p_hat * r1.M;
    ret->update_W_MIS(*p_sum);
    return ret;
}

[[nodiscard]] inline OCReservoir combine_reservoirs_MIS(OCReservoir cur_rsv,
                                                        const Geometry &geometry,
                                                        const Container<uint> &rsv_idx,
                                                        const Buffer<Reservoir> &reservoirs,
                                                        Sampler *sampler) noexcept {
    Float p_sum = 0.f;
    rsv_idx.for_each([&](const Uint &idx) {
        OCReservoir rsv = reservoirs.read(idx);
        cur_rsv->update(sampler->next_1d(), rsv->compute_weight_sum(), rsv.sample);
        cur_rsv.M += rsv.M;
        p_sum += rsv.sample.p_hat * rsv.M;
    });
    cur_rsv->update_W_MIS(p_sum);
    return cur_rsv;
}

[[nodiscard]] inline Bool is_neighbor(const OCSurfaceData &cur_surface,
                                      const OCSurfaceData &another_surface,
                                      float dot_threshold, float depth_threshold) noexcept {
    Bool cond0 = abs_dot(cur_surface->normal(), another_surface->normal()) > dot_threshold;
    Bool cond1 = (abs(cur_surface->t_max() - another_surface->t_max()) / cur_surface->t_max()) < depth_threshold;
    return cond0 && cond1 && (cur_surface.mat_id == another_surface.mat_id);
}

}// namespace vision