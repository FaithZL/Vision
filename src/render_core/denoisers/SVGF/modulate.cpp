//
// Created by Zero on 2024/3/8.
//

#include "modulate.h"
#include "svgf.h"

namespace vision::svgf {

void Modulate::prepare() noexcept {
}

void Modulate::compile() noexcept {
    Kernel kernel = [&](Var<ModulateParam> param) noexcept {
        Float3 albedo = param.albedo_buffer.read(dispatch_id()).xyz();
        Float3 emission = param.emission_buffer.read(dispatch_id()).xyz();
        SVGFDataVar svgf_data = param.svgf_buffer.read(dispatch_id());
        Float3 illumination = svgf_data->illumination();
        Float3 output = illumination * albedo + emission;
        param.output_buffer.write(dispatch_id(), make_float4(output, 1.f));
    };
    _shader = device().compile(kernel, "SVGF-modulate");
}

ModulateParam Modulate::construct_param(vision::RealTimeDenoiseInput &input) const noexcept {
    ModulateParam param;
    param.albedo_buffer = input.albedo.proxy();
    param.emission_buffer = input.emission.proxy();
    param.svgf_buffer = _svgf->cur_svgf_buffer(input.frame_index).proxy();
    param.output_buffer = input.output.proxy();
    return param;
}

CommandList Modulate::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    ret << _shader(construct_param(input))
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision::svgf