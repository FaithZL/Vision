//
// Created by Zero on 10/09/2022.
//

#pragma once

#include "node.h"
#include "encoded_object.h"
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
class Integrator : public Node, public EncodedObject {
public:
    using Desc = IntegratorDesc;
    using signature = void(uint);

protected:
    mutable uint frame_index_{};
    mutable double render_time_{};
    mutable double cur_render_time_{};
    ocarina::Shader<signature> shader_;

public:
    Integrator() = default;
    explicit Integrator(const IntegratorDesc &desc)
        : Node(desc) {}
    VS_HOTFIX_MAKE_RESTORE(Node, frame_index_, render_time_, cur_render_time_, shader_, datas_)
    virtual void compile() noexcept = 0;
    virtual Float3 Li(RayState rs, Float scatter_pdf, const HitContext &hc, const RenderEnv &render_env) const noexcept {
        return Li(rs, scatter_pdf, spectrum()->one(), hc, render_env);
    }
    virtual Float3 Li(RayState rs, Float scatter_pdf, SampledSpectrum throughput,
                      const HitContext &hc, const RenderEnv &render_env) const noexcept = 0;
    virtual Float3 Li(RayState rs, Float scatter_pdf, const Uint &max_depth, SampledSpectrum throughput,
                      bool only_direct, const HitContext &hc, const RenderEnv &render_env) const noexcept {
        OC_ERROR_FORMAT("{} Li error", typeid(*this).name());
        return make_float3(0.f);
    }
    [[nodiscard]] virtual bool has_denoiser() const noexcept { return false; }
    void upload_immediately() noexcept override {
        EncodedObject::upload_immediately();
    }
    virtual void update_resolution(uint2 res) noexcept {}
    [[nodiscard]] uint frame_index() const noexcept { return frame_index_; }
    [[nodiscard]] double render_time() const noexcept { return render_time_; }
    OC_MAKE_MEMBER_GETTER(cur_render_time, )
    void increase_frame_index() const noexcept { frame_index_++; }
    void reset_frame_index() const noexcept { frame_index_ = 0; }
    void accumulate_render_time(double ms) const noexcept {
        cur_render_time_ = ms;
        render_time_ += ms;
    }
    virtual void update_device_data() noexcept {}
    virtual void invalidation() const noexcept;
    virtual void render() const noexcept {}
};

using TIntegrator = TObject<Integrator>;

struct RenderEnv {
private:
    mutable optional<Uint> frame_index_{};
    mutable optional<SampledWavelengths> swl_{};

public:
    void reset() noexcept {
        frame_index_.reset();
        swl_.reset();
    }
    void emplace_frame_index(const Uint &val) const noexcept {
        frame_index_.emplace(val);
    }
    void emplace_sampled_wavelengths(const SampledWavelengths &val) const noexcept {
        swl_.emplace(val);
    }
    [[nodiscard]] Uint &frame_index() const noexcept { return *frame_index_; }
    [[nodiscard]] SampledWavelengths &sampled_wavelengths() const noexcept { return *swl_; }
    void initial(TSampler &sampler, const Uint &frame_index, const TSpectrum &spectrum) noexcept;
};

enum MISMode {
    EBoth = 0,
    ELight,
    EBSDF
};

class IlluminationIntegrator : public Integrator {
protected:
    EncodedData<uint> max_depth_{};
    EncodedData<uint> min_depth_{};
    EncodedData<float> rr_threshold_{};
    MISMode mis_mode_{};
    SP<ScreenBuffer> albedo_{make_shared<ScreenBuffer>("IlluminationIntegrator::albedo_")};
    SP<ScreenBuffer> emission_{make_shared<ScreenBuffer>("IlluminationIntegrator::emission_")};

    /// Material computation is separated from access memory
    bool separate_{false};
    SP<Denoiser> denoiser_{};

public:
    IlluminationIntegrator() = default;
    explicit IlluminationIntegrator(const IntegratorDesc &desc);
    VS_HOTFIX_MAKE_RESTORE(Integrator, max_depth_, min_depth_, rr_threshold_, mis_mode_,
                           albedo_, emission_, separate_, denoiser_)
    OC_ENCODABLE_FUNC(Integrator, max_depth_, min_depth_, rr_threshold_)
    VS_MAKE_GUI_STATUS_FUNC(Integrator, denoiser_)
    OC_MAKE_MEMBER_GETTER(separate, )
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] bool has_denoiser() const noexcept override { return bool(denoiser_); }
    [[nodiscard]] Float correct_bsdf_weight(Float weight, Uint bounce) const noexcept {
        switch (mis_mode_) {
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
        switch (mis_mode_) {
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

    [[nodiscard]] Float3 Li(RayState rs, Float scatter_pdf, SampledSpectrum throughput,
                            const HitContext &hc, const RenderEnv &render_env) const noexcept override;
    [[nodiscard]] Float3 Li(RayState rs, Float scatter_pdf, const Uint &max_depth, SampledSpectrum throughput,
                            bool only_direct, const HitContext &hc, const RenderEnv &render_env) const noexcept override;

    void prepare() noexcept override;

    void update_device_data() noexcept override;

    [[nodiscard]] CommandList denoise(RealTimeDenoiseInput &input) const noexcept;

    template<typename SF, typename SS>
    static SampledSpectrum direct_lighting(const Interaction &it, const SF &sf, LightSample ls,
                                           Bool occluded, TSampler &sampler,
                                           const SampledWavelengths &swl, SS &ss, bool mis = true);
};

}// namespace vision