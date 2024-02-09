//
// Created by Zero on 2024/2/8.
//

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/denoiser.h"

namespace vision {
using namespace ocarina;
class SVGF : public Denoiser {

public:
    explicit SVGF(const DenoiserDesc &desc)
        : Denoiser(desc) {
    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void apply(uint2 res, Managed<float4> *output,
               Managed<float4> *color,
               Managed<float4> *normal,
               Managed<float4> *albedo) noexcept override {
    }
    void apply(vision::DenoiseInput &input) noexcept override {

    }
    void apply(uint2 res, float4 *output, float4 *color, float4 *normal, float4 *albedo) noexcept override {
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SVGF)