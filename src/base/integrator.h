//
// Created by Zero on 10/09/2022.
//

#pragma once

#include "node.h"
#include "serial_object.h"
#include "rhi/common.h"
#include "base/scattering/interaction.h"
#include "base/denoiser.h"
#include "base/illumination/lightsampler.h"
#include "base/scattering/material.h"
#include "sampler.h"
#include "math/warp.h"

namespace vision {

class Pipeline;

class Sampler;

class RenderEnv;

//"integrator": {
//    "type": "pt",
//    "param": {
//        "min_depth": 5,
//        "max_depth": 16,
//        "rr_threshold": 1
//    }
//}
class Integrator : public Node, public SerialObject {
public:
    using Desc = IntegratorDesc;
    using signature = void(uint);

protected:
    mutable uint _host_frame_index{};
    mutable double _render_time{};
    ocarina::Shader<signature> _shader;

public:
    explicit Integrator(const IntegratorDesc &desc)
        : Node(desc) {}
    virtual void compile() noexcept = 0;
    virtual Float3 Li(RayState rs, Float scatter_pdf, const HitContext &hc, const RenderEnv &render_env) const noexcept {
        return Li(rs, scatter_pdf, spectrum().one(), hc, render_env);
    }
    virtual Float3 Li(RayState rs, Float scatter_pdf, SampledSpectrum throughput,
                      const HitContext &hc, const RenderEnv &render_env) const noexcept = 0;
    virtual Float3 Li(RayState rs, Float scatter_pdf, const Uint &max_depth, SampledSpectrum throughput,
                      bool only_direct, const HitContext &hc, const RenderEnv &render_env) const noexcept {
        OC_ERROR_FORMAT("{} Li error", typeid(*this).name());
        return make_float3(0.f);
    }
    [[nodiscard]] uint host_frame_index() const noexcept { return _host_frame_index; }
    [[nodiscard]] double render_time() const noexcept { return _render_time; }
    void increase_frame_index() const noexcept { _host_frame_index++; }
    void reset_frame_index() const noexcept { _host_frame_index = 0; }
    void accumulate_render_time(double ms) const noexcept { _render_time += ms; }
    virtual void invalidation() const noexcept;
    virtual void render() const noexcept {}
};

struct RenderEnv {
private:
    mutable optional<Uint> _frame_index{};
    mutable optional<SampledWavelengths> _swl{};

public:
    void reset() noexcept {
        _frame_index.reset();
        _swl.reset();
    }
    void emplace_frame_index(const Uint &val) const noexcept {
        _frame_index.emplace(val);
    }
    void emplace_sampled_wavelengths(const SampledWavelengths &val) const noexcept {
        _swl.emplace(val);
    }
    [[nodiscard]] Uint &frame_index() const noexcept { return *_frame_index; }
    [[nodiscard]] SampledWavelengths &sampled_wavelengths() const noexcept { return *_swl; }
    void initial(Sampler *sampler, const Uint &frame_index, const Spectrum &spectrum) noexcept;
};

enum MISMode {
    EBoth = 0,
    ELight,
    EBSDF
};

class IlluminationIntegrator : public Integrator {
protected:
    Serial<uint> _max_depth{};
    Serial<uint> _min_depth{};
    Serial<float> _rr_threshold{};
    MISMode _mis_mode{};
    RegistrableBuffer<PixelData> _pixel_data{};
    /// Material computation is separated from access memory
    bool _separate{false};
    SP<Denoiser> _denoiser{};

public:
    explicit IlluminationIntegrator(const IntegratorDesc &desc);

    OC_SERIALIZABLE_FUNC(Integrator, _max_depth, _min_depth, _rr_threshold)

    OC_MAKE_MEMBER_GETTER(separate, )
    OC_MAKE_MEMBER_GETTER(pixel_data, &)

    [[nodiscard]] Float correct_bsdf_weight(Float weight, Uint bounce) const noexcept {
        switch (_mis_mode) {
            case MISMode::EBSDF: {
                weight = 1.f;
                break;
            }
            case MISMode::ELight: {
                weight = ocarina::select(bounce == 0, weight, 0.f);
                break;
            }
            default: break;
        }
        return weight;
    };

    template<typename... Args>
    [[nodiscard]] SampledSpectrum direct_light_mis(Args &&...args) const noexcept {
        switch (_mis_mode) {
            case MISMode::EBSDF: {
                return direct_lighting(OC_FORWARD(args)...) * 0.f;
            }
            case MISMode::ELight: {
                return direct_lighting(OC_FORWARD(args)..., false);
            }
            default: break;
        }
        return direct_lighting(OC_FORWARD(args)...);
    };

    [[nodiscard]] SampledSpectrum evaluate_miss(RayState &rs, const Float3 &normal,
                                                const Float &scatter_pdf, const Uint &bounces,
                                                const SampledWavelengths &swl) const noexcept;

    [[nodiscard]] static Float2 compute_motion_vec(const Float2 &p_film, const Float3 &cur_pos,
                                                   const Bool &is_hit) noexcept;

    [[nodiscard]] Float3 Li(RayState rs, Float scatter_pdf, SampledSpectrum throughput, const HitContext &hc, const RenderEnv &render_env) const noexcept override;
    [[nodiscard]] Float3 Li(RayState rs, Float scatter_pdf, const Uint &max_depth, SampledSpectrum throughput,
                            bool only_direct, const HitContext &hc, const RenderEnv &render_env) const noexcept override;

    void prepare() noexcept override;

    [[nodiscard]] CommandList denoise(DenoiseInput &input) noexcept;

    template<typename SF, typename SS>
    static SampledSpectrum direct_lighting(const Interaction &it, const SF &sf, LightSample ls,
                                           Bool occluded, Sampler *sampler,
                                           const SampledWavelengths &swl, SS &ss, bool mis = true) {
        Float3 wi = normalize(ls.p_light - it.pos);
        ScatterEval scatter_eval = sf.evaluate(it.wo, wi);
        ss = sf.sample(it.wo, sampler);
        Bool is_delta_light = ls.eval.pdf < 0;
        Float weight = mis ? (select(is_delta_light, 1.f, vision::MIS_weight<D>(ls.eval.pdf, scatter_eval.pdf))) : 1.f;
        ls.eval.pdf = select(is_delta_light, -ls.eval.pdf, ls.eval.pdf);
        SampledSpectrum Ld = {swl.dimension(), 0.f};
        $if(!occluded && scatter_eval.valid() && ls.valid()) {
            Ld = ls.eval.L * scatter_eval.f * weight / ls.eval.pdf;
        };
        return Ld;
    }
};

class BufferMgr {
protected:
    RegistrableBuffer<float2> _motion_vectors{};
    RegistrableBuffer<SurfaceData> _surfaces{};
    RegistrableBuffer<HitBSDF> _hit_bsdfs{};
    RegistrableBuffer<float3> _radiance0;
    RegistrableBuffer<float3> _radiance1;

public:
    BufferMgr();
    OC_MAKE_MEMBER_GETTER(motion_vectors, &)
    OC_MAKE_MEMBER_GETTER(surfaces, &)
    OC_MAKE_MEMBER_GETTER(hit_bsdfs, &)
    OC_MAKE_MEMBER_GETTER(radiance0, &)
    OC_MAKE_MEMBER_GETTER(radiance1, &)
};

class RayTracingIntegrator : public BufferMgr, public IlluminationIntegrator {
public:
    using IlluminationIntegrator::IlluminationIntegrator;
};

}// namespace vision