//
// Created by Zero on 2023/5/30.
//

#include "base/denoiser.h"

namespace vision {

class OptixDenoiser : public Denoiser {
public:
    explicit OptixDenoiser(const DenoiserDesc &desc)
        : Denoiser(desc){
    }

    void apply(uint2 res, float4 *output, float4 *color,
               float4 *normal, float4 *albedo) noexcept override {
    }

    void apply(uint2 res, Managed<float4> *output,
               Managed<float4> *color,
               Managed<float4> *normal,
               Managed<float4> *albedo) noexcept override {

    }
    void apply(vision::DenoiseInput &input) noexcept override {

    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OptixDenoiser)