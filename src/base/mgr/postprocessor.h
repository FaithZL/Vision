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
    Pipeline *_rp{};
    SP<Denoiser> _denoiser{};
    SP<ToneMapper> _tone_mapper{};
    ocarina::Shader<signature> _tone_mapping_shader;

public:
    explicit Postprocessor(Pipeline *rp);
    void set_denoiser(SP<Denoiser> denoiser) noexcept { _denoiser = denoiser; }
    void set_tone_mapper(SP<ToneMapper> tone_mapper) noexcept {
        if (_tone_mapper && tone_mapper->type_hash() == _tone_mapper->type_hash()) {
            return;
        }
        _tone_mapper = tone_mapper;
        compile_tone_mapping();
    }
    void compile_tone_mapping() noexcept;
    template<typename... Args>
    void denoise(Args &&...args) const noexcept {
        _denoiser->apply(OC_FORWARD(args)...);
    }
    void tone_mapping(RegistrableManaged<float4> &input,
                      RegistrableManaged<float4> &output,
                      bool gamma = false) noexcept;
};
}// namespace vision