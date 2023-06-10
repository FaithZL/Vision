//
// Created by Zero on 2023/6/1.
//

#pragma once

#include "base/denoiser.h"
#include "base/tonemapping.h"

namespace vision {
class RenderPipeline;
class Postprocessor {
private:
    RenderPipeline *_rp{};
    Denoiser *_denoiser{};
    ToneMapper *_tone_mapper{};

public:
    explicit Postprocessor(RenderPipeline *rp);
    void set_denoiser(Denoiser *denoiser) noexcept { _denoiser = denoiser; }
    void set_tone_mapper(ToneMapper *tone_mapper) noexcept { _tone_mapper = tone_mapper; }
    template<typename ...Args>
    void denoise(Args &&...args) const noexcept {
        _denoiser->apply(OC_FORWARD(args)...);
    }
};
}// namespace vision