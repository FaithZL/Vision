//
// Created by Zero on 23/01/2023.
//

#include "texture.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

SampledSpectrum Texture::eval_albedo_spectrum(const TextureEvalContext &tec, const SampledWavelengths &swl) const noexcept {
    Float3 rgb = eval(tec).xyz();
    return render_pipeline()->spectrum().decode_to_albedo(rgb, swl).value;
}

SampledSpectrum Texture::eval_illumination_spectrum(const TextureEvalContext &tec, const SampledWavelengths &swl) const noexcept {
    Float3 rgb = eval(tec).xyz();
    return render_pipeline()->spectrum().decode_to_illumination(rgb, swl).value;
}

SampledSpectrum Texture::eval_albedo_spectrum(const Float2 &uv, const SampledWavelengths &swl) const noexcept {
    Float3 rgb = eval(uv).xyz();
    return render_pipeline()->spectrum().decode_to_albedo(rgb, swl).value;
}

SampledSpectrum Texture::eval_illumination_spectrum(const Float2 &uv, const SampledWavelengths &swl) const noexcept {
    Float3 rgb = eval(uv).xyz();
    return render_pipeline()->spectrum().decode_to_illumination(rgb, swl).value;
}

}// namespace vision