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
    uint _dim{4};

public:
    using Desc = ShaderNodeDesc;

public:
    explicit ShaderNode(const ShaderNodeDesc &desc) : Node(desc), _type(desc.type) {}
    [[nodiscard]] virtual uint dim() const noexcept { return _dim; }
    [[nodiscard]] virtual bool is_zero() const noexcept { return false; }
    /**
     * if shader node is constant, the result will be inlined
     * @return
     */
    [[nodiscard]] virtual bool is_constant() const noexcept { return false; }
    /**
     * if shader node contain textures,the result is versatile
     * @return
     */
    [[nodiscard]] virtual bool is_versatile() const noexcept { return true; }
    [[nodiscard]] virtual Float4 eval(const AttrEvalContext &ctx) const noexcept = 0;
    [[nodiscard]] virtual ColorDecode eval_albedo_spectrum(const AttrEvalContext &tec,
                                                           const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_unbound_spectrum(const AttrEvalContext &ctx,
                                                            const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_illumination_spectrum(const AttrEvalContext &tec,
                                                                 const SampledWavelengths &swl) const noexcept;
    virtual void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};

template<uint Dim = 1>
requires(Dim <= 4) class TSlot : public ocarina::Hashable {
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
    TSlot() = default;

    explicit TSlot(const ShaderNode *input, string channels)
        : _node(input),
          _channel_mask(_calculate_mask(channels)) {
        OC_ASSERT(channels.size() == Dim);
    }

    [[nodiscard]] bool is_zero() const noexcept { return _node->is_zero(); }
    [[nodiscard]] bool is_constant() const noexcept { return _node->is_constant(); }
    [[nodiscard]] bool is_versatile() const noexcept { return _node->is_versatile(); }

    [[nodiscard]] auto node() const noexcept { return _node; }
    [[nodiscard]] auto node() noexcept { return _node; }

    [[nodiscard]] ColorDecode eval_albedo_spectrum(const AttrEvalContext &ctx,
                                                   const SampledWavelengths &swl) const noexcept {
        static_assert(Dim == 3, "eval_albedo_spectrum dim must be 3!");
        Float3 val = eval(ctx);
        return _node->spectrum().decode_to_albedo(val, swl);
    }

    [[nodiscard]] ColorDecode eval_unbound_spectrum(const AttrEvalContext &ctx,
                                                    const SampledWavelengths &swl) const noexcept {
        static_assert(Dim == 3, "eval_unbound_spectrum dim must be 3!");
        Float3 val = eval(ctx);
        return _node->spectrum().decode_to_unbound_spectrum(val, swl);
    }

    [[nodiscard]] ColorDecode eval_illumination_spectrum(const AttrEvalContext &ctx,
                                                         const SampledWavelengths &swl) const noexcept {
        static_assert(Dim == 3, "eval_illumination_spectrum dim must be 3!");
        Float3 val = eval(ctx);
        return _node->spectrum().decode_to_illumination(val, swl);
    }

    [[nodiscard]] auto eval(const AttrEvalContext &ctx) const noexcept {
        if constexpr (Dim == 1) {
            switch (_channel_mask) {
                case 0x0: return _node->eval(ctx).x;
                case 0x1: return _node->eval(ctx).y;
                case 0x2: return _node->eval(ctx).z;
                case 0x3: return _node->eval(ctx).w;
                default: OC_ASSERT(0); return Var(0.f);
            }
        } else if constexpr (Dim == 2) {
            switch (_channel_mask) {
#include "slot_swizzle_2.inl.h"
                default: OC_ASSERT(0); return Var(make_float2(0.f));
            }
        } else if constexpr (Dim == 3) {
            switch (_channel_mask) {
#include "slot_swizzle_3.inl.h"
                default: OC_ASSERT(0); return Var(make_float3(0.f));
            }
        } else {
            switch (_channel_mask) {
#include "slot_swizzle_4.inl.h"
                default: OC_ASSERT(0); return Var(make_float4(0.f));
            }
        }
    }
};

}// namespace vision