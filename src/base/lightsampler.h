//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"
#include "core/stl.h"
#include "light.h"
#include "base/scattering/interaction.h"

namespace vision {
using namespace ocarina;

struct SampledLight {
    Uint light_id;
    Float PMF;
};

class Sampler;

class LightSampler : public Node {
public:
    using Desc = LightSamplerDesc;

protected:
    Polymorphic<Light *> _lights;
    Light *_env_light{};
    float _env_prob{};

public:
    explicit LightSampler(const LightSamplerDesc &desc);
    void prepare() noexcept override;
    template<typename... Args>
    void set_mode(Args &&...args) noexcept {
        _lights.set_mode(OC_FORWARD(args)...);
    }
    [[nodiscard]] const Light *env_light() const noexcept { return _env_light; }
    [[nodiscard]] const Polymorphic<Light *> &lights() const noexcept { return _lights; }
    [[nodiscard]] Polymorphic<Light *> &lights() noexcept { return _lights; }
    [[nodiscard]] uint light_num() const noexcept { return _lights.size(); }
    void add_light(Light *light) noexcept { _lights.push_back(light); }
    [[nodiscard]] virtual Float PMF(const LightSampleContext &lsc, const Uint &id) const noexcept = 0;
    [[nodiscard]] virtual LightEval evaluate_hit(const LightSampleContext &p_ref, const Interaction &it,
                                                 const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual LightEval evaluate_miss(const LightSampleContext &p_ref, Float3 wi,
                                                  const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual SampledLight select_light(const LightSampleContext &lsc, const Float &u) const noexcept = 0;
    [[nodiscard]] virtual LightSample sample(const LightSampleContext &lsc, Sampler *sampler,
                                             const SampledWavelengths &swl) const noexcept = 0;
    void dispatch_light(const Uint &id, const std::function<void(const Light *)> &func) const noexcept;
    template<typename Func>
    void for_each(Func &&func) noexcept {
        for (Light *light : _lights) {
            func(light);
        }
    }

    template<typename Func>
    void for_each(Func &&func) const noexcept {
        for (const Light *light : _lights) {
            func(light);
        }
    }
};
}// namespace vision