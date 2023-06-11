//
// Created by Zero on 2023/5/30.
//

#pragma once

#include "dsl/common.h"
#include "node.h"

namespace vision {

class Denoiser : public Node {
public:
    enum Mode {
        RT = 0,
        RTLightmap = 1
    };

    enum Backend {
        GPU = 0,
        CPU = 1
    };

protected:
    Mode _mode{};
    Backend _backend{};

public:
    using Desc = DenoiserDesc;

public:
    explicit Denoiser(const DenoiserDesc &desc)
        : Node(desc),
          _mode(RT),
          _backend(to_upper(desc["backend"].as_string()) == "CPU" ? CPU : GPU) {}
    virtual void apply(uint2 res, float4 *output, float4 *color,
                       float4 *normal, float4 *albedo) noexcept = 0;
    virtual void apply(uint2 res, RegistrableManaged<float4> &output, RegistrableManaged<float4> &color,
                       RegistrableManaged<float4> *normal, RegistrableManaged<float4> *albedo) noexcept = 0;
    [[nodiscard]] Backend backend() const noexcept { return _backend; }
};

}// namespace vision