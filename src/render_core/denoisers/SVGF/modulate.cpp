//
// Created by Zero on 2024/3/8.
//

#include "modulate.h"
#include "svgf.h"

namespace vision {

void Modulate::prepare() noexcept {
}

void Modulate::compile() noexcept {
    Kernel kernel = [&](BufferVar<float4> albedo_buffer, BufferVar<float4> emission_buffer,
                        BufferVar<SVGFData> svgf_buffer, BufferVar<float4> output_buffer) noexcept {
        Float3 albedo = albedo_buffer.read(dispatch_id()).xyz();
        Float3 emission = emission_buffer.read(dispatch_id()).xyz();
        SVGFDataVar svgf_data = svgf_buffer.read(dispatch_id());
        Float3 illumination = svgf_data->illumination();
        Float3 output = illumination * albedo + emission;
        output_buffer.write(dispatch_id(), make_float4(output, 1.f));
    };
    _shader = device().compile(kernel, "SVGF-modulate");
}

CommandList Modulate::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    ret << _shader(input.albedo, input.emission,
                   _svgf->cur_svgf_buffer(input.frame_index), input.output)
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision