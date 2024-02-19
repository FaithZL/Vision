//
// Created by Zero on 2023/5/30.
//

#pragma once

#include "dsl/dsl.h"
#include "node.h"
#include "frame_buffer.h"

namespace vision {

struct OfflineDenoiseInput {
    uint2 resolution{};
    uint frame_index{};

    Managed<float4> *output{};
    Managed<float4> *color{};
    Managed<float4> *position{};
    Managed<float4> *normal{};
    Managed<float4> *albedo{};
};

struct RealTimeDenoiseInput {
    uint2 resolution{};
    uint frame_index{};

    BufferView<float4> output;
    BufferView<float4> radiance;
    BufferView<float4> albedo;
    BufferView<float4> emission;
    BufferView<PixelGeometry> gbuffer;
    BufferView<PixelGeometry> prev_gbuffer;
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
    virtual void apply(OfflineDenoiseInput &input) noexcept {
        OC_ERROR_FORMAT("denoiser {} error apply", typeid(*this).name());
    }

    /// for real time denoise
    virtual CommandList dispatch(RealTimeDenoiseInput &input) noexcept {
        OC_ERROR_FORMAT("denoiser {} error dispatch", typeid(*this).name());
        return {};
    }
    [[nodiscard]] Backend backend() const noexcept { return _backend; }
};

}// namespace vision