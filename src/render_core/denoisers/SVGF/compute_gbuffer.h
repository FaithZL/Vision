//
// Created by Zero on 2024/2/16.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/sensor/filter.h"
#include "base/denoiser.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"

namespace vision {

class SVGF;

class ComputeGBuffer : public Ctx {
private:
    SVGF *_svgf{nullptr};
    Shader<void(uint, Buffer<PixelGeometry>)> _compute_geometry;
    Shader<void(uint, Buffer<PixelGeometry>)> _compute_gradiant;

public:
    explicit ComputeGBuffer(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    void compile_compute_geom() noexcept;
    void compile_compute_grad() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(DenoiseInput &input) noexcept;
};
}// namespace vision