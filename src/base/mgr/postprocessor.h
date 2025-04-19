//
// Created by Zero on 2023/6/1.
//

#pragma once

#include "base/denoiser.h"
#include "base/sensor/tonemapper.h"

namespace vision {
class Pipeline;
class Postprocessor {
private:
    using signature = void(Buffer<float4>, Buffer<float4>, bool);

private:
    Pipeline *rp_{};
    SP<Denoiser> denoiser_{};
    TToneMapper tone_mapper_{};
    ocarina::Shader<signature> tone_mapping_shader_;

public:
    explicit Postprocessor(Pipeline *rp);
    void set_denoiser(SP<Denoiser> denoiser) noexcept { denoiser_ = denoiser; }
    void set_tone_mapper(const TToneMapper & tone_mapper) noexcept {
        if (tone_mapper_ && tone_mapper->topology_hash() == tone_mapper_->topology_hash()) {
            return;
        }
        tone_mapper_ = tone_mapper;
        compile_tone_mapping();
    }
    void compile_tone_mapping() noexcept;
    template<typename... Args>
    void denoise(Args &&...args) const noexcept {
        denoiser_->apply(OC_FORWARD(args)...);
    }
    void tone_mapping(RegistrableManaged<float4> &input,
                      RegistrableManaged<float4> &output,
                      bool gamma = false) noexcept;
};
}// namespace vision