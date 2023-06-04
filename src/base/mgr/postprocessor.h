//
// Created by Zero on 2023/6/1.
//

#pragma once

#include "base/denoiser.h"

namespace vision {
class Postprocessor {
private:
    Denoiser *_denoiser{};
public:
    Postprocessor() = default;
    void set_denoiser(Denoiser *denoiser) noexcept { _denoiser = denoiser; }
};
}