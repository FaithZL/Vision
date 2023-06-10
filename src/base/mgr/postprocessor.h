//
// Created by Zero on 2023/6/1.
//

#pragma once

#include "base/denoiser.h"
#include "base/tonemapping.h"

namespace vision {
class Postprocessor {
private:
    Denoiser *_denoiser{};
    ToneMapper *_tone_mapping{};

public:
    Postprocessor() = default;
    void set_denoiser(Denoiser *denoiser) noexcept { _denoiser = denoiser; }
    void set_tone_mapping(ToneMapper *tone_mapping) noexcept { _tone_mapping = tone_mapping; }
};
}// namespace vision