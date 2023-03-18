//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/node.h"
#include "util/image_io.h"
#include "base/color/spectrum.h"
#include "base/scattering/interaction.h"

namespace vision {

class ShaderNode : public Node {
protected:
    ShaderNodeType _type{};

public:
    using Desc = ShaderNodeDesc;

public:
    [[nodiscard]] static bool is_zero(const ShaderNode *tex) noexcept {
        return tex ? tex->is_zero() : true;
    }
    [[nodiscard]] static bool nonzero(const ShaderNode *tex) noexcept {
        return !is_zero(tex);
    }

public:
    explicit ShaderNode(const ShaderNodeDesc &desc) : Node(desc), _type(desc.type) {}
    [[nodiscard]] virtual bool is_zero() const noexcept { return false; }
    [[nodiscard]] virtual Float4 eval(const AttrEvalContext &tec) const noexcept = 0;
    [[nodiscard]] virtual Float4 eval(const Float2 &uv) const noexcept {
        return eval(AttrEvalContext(uv));
    }
    [[nodiscard]] virtual ColorDecode eval_albedo_spectrum(const AttrEvalContext &tec,
                                                           const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_illumination_spectrum(const AttrEvalContext &tec,
                                                                 const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_albedo_spectrum(const Float2 &uv,
                                                           const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_illumination_spectrum(const Float2 &uv,
                                                                 const SampledWavelengths &swl) const noexcept;
    virtual void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};

template<uint Dim = 1>
requires(Dim <= 4) class Slot : public ocarina::Hashable {
private:
    uint _channel_mask{};
    const ShaderNode *_node{};

private:
    [[nodiscard]] static uint _calculate_mask(string channels) noexcept {
        OC_ASSERT(channels.size() == Dim);
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

    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(_channel_mask, _node->hash());
    }

    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_channel_mask, _node->type_hash());
    }

public:
    Slot() = default;

    explicit Slot(const ShaderNode *input, string channels)
        : _node(input),
          _channel_mask(_calculate_mask(channels)) {
        OC_ASSERT(channels.size() == Dim);
    }

    [[nodiscard]] auto eval(const AttrEvalContext &ctx) const noexcept {
        if constexpr (Dim == 1) {
            switch (_channel_mask) {
                case 0x0: return _node->eval(ctx).x;
                case 0x1: return _node->eval(ctx).y;
                case 0x2: return _node->eval(ctx).z;
                case 0x3: return _node->eval(ctx).w;
                default: OC_ASSERT(0); return 0;
            }
        } else if constexpr (Dim == 2) {
            switch (_channel_mask) {
#include "slot_swizzle_2.inl.h"
                default: OC_ASSERT(0); return make_float2(0.f);
            }
        } else if constexpr (Dim == 3) {
            switch (_channel_mask) {
#include "slot_swizzle_3.inl.h"
                default: OC_ASSERT(0); return make_float3(0.f);
            }
        } else {
            switch (_channel_mask) {
#include "slot_swizzle_4.inl.h"
                default: OC_ASSERT(0); return make_float4(0.f);
            }
        }
    }
};

}// namespace vision