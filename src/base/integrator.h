//
// Created by Zero on 10/09/2022.
//

#pragma once

#include "node.h"
#include "serial_object.h"
#include "rhi/common.h"
#include "base/scattering/interaction.h"
#include "base/illumination/lightsampler.h"
#include "base/scattering/material.h"
#include "sampler.h"
#include "math/warp.h"

namespace vision {

class Pipeline;

class Sampler;

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
    using signature = void(uint, DebugData);

protected:
    mutable uint _frame_index{};
    mutable double _render_time{};
    mutable int2 _debug_pixel{make_int2(-1)};
    mutable DebugData _debug_data;
    ocarina::Shader<signature> _shader;

public:
    explicit Integrator(const IntegratorDesc &desc)
        : Node(desc) {}
    virtual void compile() noexcept = 0;
    virtual Float3 Li(RayState rs, Float scatter_pdf, Interaction *it) const noexcept = 0;
    [[nodiscard]] uint frame_index() const noexcept { return _frame_index; }
    [[nodiscard]] double render_time() const noexcept { return _render_time; }
    void increase_frame_index() const noexcept { _frame_index++; }
    void reset_frame_index() const noexcept { _frame_index = 0; }
    void set_debug_pixel(int2 pixel) const noexcept { _debug_pixel = pixel; }
    void reset_debug_pixel() const noexcept { _debug_pixel = make_int2(-1); }
    void accumulate_render_time(double ms) const noexcept { _render_time += ms; }
    virtual void invalidation() const noexcept {
        _frame_index = 0u;
        _render_time = 0;
    }
    virtual void render() const noexcept {}
};

class IlluminationIntegrator : public Integrator {
protected:
    Serial<uint> _max_depth{};
    Serial<uint> _min_depth{};
    Serial<float> _rr_threshold{};

public:
    explicit IlluminationIntegrator(const IntegratorDesc &desc)
        : Integrator(desc),
          _max_depth(desc["max_depth"].as_uint(16)),
          _min_depth(desc["min_depth"].as_uint(5)),
          _rr_threshold(desc["rr_threshold"].as_float(1.f)) {}

    OC_SERIALIZABLE_FUNC(Integrator, _max_depth, _min_depth, _rr_threshold)

    [[nodiscard]] Float3 Li(RayState rs, Float scatter_pdf, Interaction *first_it) const noexcept override;

    void prepare() noexcept override {
        encode_data();
        datas().reset_device_buffer_immediately(device());
        datas().register_self();
        datas().upload_immediately();
    }

    template<typename SF, typename SS>
    static SampledSpectrum direct_lighting(Interaction it, const SF &sf, LightSample ls,
                                           Bool occluded, Sampler *sampler,
                                           const SampledWavelengths &swl, SS &ss) {
        Float3 wi = normalize(ls.p_light - it.pos);
        ScatterEval scatter_eval = sf.evaluate(it.wo, wi);
        ss = sf.sample(it.wo, sampler);
        Bool is_delta_light = ls.eval.pdf < 0;
        Float weight = select(is_delta_light, 1.f, mis_weight<D>(ls.eval.pdf, scatter_eval.pdf));
        ls.eval.pdf = select(is_delta_light, -ls.eval.pdf, ls.eval.pdf);
        SampledSpectrum Ld = {swl.dimension(), 0.f};
        $if(!occluded && scatter_eval.valid() && ls.valid()) {
            Ld = ls.eval.L * scatter_eval.f * weight / ls.eval.pdf;
        };
        return Ld;
    }
};

}// namespace vision