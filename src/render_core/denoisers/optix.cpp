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
    void apply(vision::DenoiseInput &input) noexcept override {

    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OptixDenoiser)