//
// Created by Zero on 23/01/2023.
//

#include "shader_node.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

ColorDecode ShaderNode::eval_albedo_spectrum(const AttrEvalContext &ctx,
                                             const SampledWavelengths &swl) const noexcept {
    Float3 rgb = eval(ctx).xyz();
    return spectrum().decode_to_albedo(rgb, swl);
}

ColorDecode ShaderNode::eval_unbound_spectrum(const AttrEvalContext &ctx,
                                              const SampledWavelengths &swl) const noexcept {
    Float3 rgb = eval(ctx).xyz();
    return spectrum().decode_to_unbound_spectrum(rgb, swl);
}

ColorDecode ShaderNode::eval_illumination_spectrum(const AttrEvalContext &ctx,
                                                   const SampledWavelengths &swl) const noexcept {
    Float3 rgb = eval(ctx).xyz();
    return spectrum().decode_to_illumination(rgb, swl);
}

}// namespace vision