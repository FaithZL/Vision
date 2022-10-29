//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"
#include "core/stl.h"
#include "light.h"
#include "interaction.h"

namespace vision {
using namespace ocarina;

struct SampledLight {
    Uint light_id;
    Float PMF;
};

class LightSampler : public Node {
public:
    using Desc = LightSamplerDesc;

protected:
    vector<Light *> _lights;

public:
    explicit LightSampler(const LightSamplerDesc &desc);
    [[nodiscard]] span<const Light *const> lights() const noexcept { return _lights; }
    [[nodiscard]] uint light_num() const noexcept { return _lights.size(); }
    [[nodiscard]] virtual Float PMF(const Uint &id) const noexcept = 0;
    void add_light(Light *light) noexcept { _lights.push_back(light); }
    [[nodiscard]] virtual Float PMF(const LightSampleContext &lsc, const Uint &id) const noexcept {
        return PMF(id);
    }
    [[nodiscard]] virtual SampledLight select_light(const Float &u) const noexcept = 0;
    [[nodiscard]] virtual SampledLight select_light(const LightSampleContext &lsc, const Float& u) const noexcept {
        return select_light(u);
    }
    [[nodiscard]] virtual LightSample sample(const Float& u_light, const Float& u_surface) const noexcept = 0;
    [[nodiscard]] virtual LightSample sample(const LightSampleContext &lsc, const Float& u_light,
                                             const Float& u_surface) const noexcept {
        return sample(u_light, u_surface);
    }
    template<typename Func>
    void for_each(Func &&func) noexcept {
        for (Light *light : _lights) {
            func(light);
        }
    }
};
}// namespace vision