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
    void apply(vision::OfflineDenoiseInput &input) noexcept override {

    }
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OptixDenoiser)