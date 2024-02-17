//
// Created by Zero on 2023/5/30.
//

#pragma once

#include "dsl/dsl.h"
#include "node.h"
#include "gbuffer.h"

namespace vision {

struct DenoiseInput {
    uint2 resolution{};
    uint frame_index{};

    // real time denoise
    Buffer<PixelGeometry> *prev_gbuffer{};
    Buffer<PixelGeometry> *gbuffer{};
    Buffer<float4> *radiance{};
    Buffer<float4> *gpu_output{};

    // offline denoise
    Managed<float4> *output{};
    Managed<float4> *color{};
    Managed<float4> *position{};
    Managed<float4> *normal{};
    Managed<float4> *albedo{};
};

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

    virtual void compile() noexcept {}

    /// for offline denoise
    virtual void apply(DenoiseInput &input) noexcept {
        OC_ERROR_FORMAT("denoiser {} error apply", typeid(*this).name());
    }

    /// for real time denoise
    virtual CommandList dispatch(DenoiseInput &input) noexcept {
        OC_ERROR_FORMAT("denoiser {} error dispatch", typeid(*this).name());
        return {};
    }
    [[nodiscard]] Backend backend() const noexcept { return _backend; }
};

}// namespace vision