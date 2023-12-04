//
// Created by Zero on 2023/6/1.
//

#include "postprocessor.h"
#include "pipeline.h"

namespace vision {

Postprocessor::Postprocessor(Pipeline *rp)
    : _rp(rp) {}

void Postprocessor::compile_tone_mapping() noexcept {
    Kernel<signature> kernel = [&](BufferVar<float4> input, BufferVar<float4> output) {
        Float4 input_pixel = input.read(dispatch_id());
        Float4 output_pixel = linear_to_srgb(_tone_mapper->apply(input_pixel));
        output_pixel.w = 1.f;
        output.write(dispatch_id(), output_pixel);
    };
    _tone_mapping_shader = _rp->device().compile(kernel, "tonemapping");
}
void Postprocessor::tone_mapping(RegistrableManaged<float4> &input, RegistrableManaged<float4> &output) noexcept {
    Stream &stream = _rp->stream();
    stream << _tone_mapping_shader(input.device_buffer(), output.device_buffer()).dispatch(_rp->resolution());
    stream << synchronize();
    stream << commit();
}

}// namespace vision