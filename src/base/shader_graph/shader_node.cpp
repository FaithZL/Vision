//
// Created by Zero on 23/01/2023.
//

#include "shader_node.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

Array<float> ShaderNode::evaluate(const AttrEvalContext &ctx,
                                  const DataAccessor &da) const noexcept {
    OC_ASSERT(false);
    return Array<float>();
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

uint64_t Slot::_compute_hash() const noexcept {
    return hash64(_channel_mask, _dim, _node->hash());
}

uint64_t Slot::_compute_type_hash() const noexcept {
    return hash64(_channel_mask, _dim, _node->type_hash());
}

Array<float> Slot::evaluate(const AttrEvalContext &ctx) const noexcept {
    switch (_dim) {
        case 1: {
            switch (_channel_mask) {
#include "slot_swizzle_1.inl.h"
            }
        }
        case 2: {
            switch (_channel_mask) {
#include "slot_swizzle_2.inl.h"
            }
        }
        case 3: {
            switch (_channel_mask) {
#include "slot_swizzle_3.inl.h"
            }
        }
        case 4: {
            switch (_channel_mask) {
#include "slot_swizzle_4.inl.h"
            }
        }
    }
    OC_ASSERT(false);
    return _node->evaluate(ctx);
}

Array<float> Slot::evaluate(const AttrEvalContext &ctx,
                            DataAccessor &da) const noexcept {
    switch (_dim) {
        case 1: {
            switch (_channel_mask) {
#include "slot_swizzle1.inl.h"
            }
        }
        case 2: {
            switch (_channel_mask) {
#include "slot_swizzle2.inl.h"
            }
        }
        case 3: {
            switch (_channel_mask) {
#include "slot_swizzle3.inl.h"
            }
        }
        case 4: {
            switch (_channel_mask) {
#include "slot_swizzle4.inl.h"
            }
        }
    }
    OC_ASSERT(false);
    return _node->evaluate(ctx, da);
}

ColorDecode Slot::eval_albedo_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(_dim == 3);
    Float3 val = evaluate(ctx).to_vec3();
    return _node->spectrum().decode_to_albedo(val, swl);
}

ColorDecode Slot::eval_unbound_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(_dim == 3);
    Float3 val = evaluate(ctx).to_vec3();
    return _node->spectrum().decode_to_unbound_spectrum(val, swl);
}

ColorDecode Slot::eval_illumination_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(_dim == 3);
    Float3 val = evaluate(ctx).to_vec3();
    return _node->spectrum().decode_to_illumination(val, swl);
}

ColorDecode Slot::eval_albedo_spectrum(const AttrEvalContext &ctx,
                                       DataAccessor &da,
                                       const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(_dim == 3);
    Float3 val = evaluate(ctx, da).to_vec3();
    return _node->spectrum().decode_to_albedo(val, swl);
}

ColorDecode Slot::eval_unbound_spectrum(const AttrEvalContext &ctx,
                                        DataAccessor &da,
                                        const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(_dim == 3);
    Float3 val = evaluate(ctx, da).to_vec3();
    return _node->spectrum().decode_to_unbound_spectrum(val, swl);
}

ColorDecode Slot::eval_illumination_spectrum(const AttrEvalContext &ctx,
                                             DataAccessor &da,
                                             const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(_dim == 3);
    Float3 val = evaluate(ctx, da).to_vec3();
    return _node->spectrum().decode_to_illumination(val, swl);
}

}// namespace vision