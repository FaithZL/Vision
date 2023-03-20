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

uint Slot::_calculate_mask(string channels) noexcept {
    uint ret{};
    channels = to_lower(channels);
    static map<char, uint> dict{
        {'x', 0u},
        {'y', 1u},
        {'z', 2u},
        {'w', 3u},
        {'r', 0u},
        {'g', 1u},
        {'b', 2u},
        {'a', 3u},
    };
    for (char channel : channels) {
        ret = (ret << 4) | dict[channel];
    }
    return ret;
}

}// namespace vision