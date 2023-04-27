//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/node.h"
#include "util/image_io.h"
#include "base/color/spectrum.h"
#include "base/scattering/interaction.h"

namespace vision {

class ShaderNode : public Node, public ISerializable<float> {
protected:
    ShaderNodeType _type{};
    mutable Array<float> _value_ref{};

public:
    using Desc = ShaderNodeDesc;

public:
    explicit ShaderNode(const ShaderNodeDesc &desc) : Node(desc), _type(desc.type) {}
    [[nodiscard]] virtual uint dim() const noexcept { return 4; }
    [[nodiscard]] ShaderNodeType type() const noexcept { return _type; }
    [[nodiscard]] virtual bool is_zero() const noexcept { return false; }
    /**
     * if shader node is constant, the result will be inlined
     * @return
     */
    [[nodiscard]] virtual bool is_constant() const noexcept { return false; }
    /**
     * if shader node contain textures, the result is not uniform
     * @return
     */
    [[nodiscard]] virtual bool is_uniform() const noexcept { return false; }
    /**
     * data size in byte
     * @return
     */
    virtual Array<float> evaluate(const AttrEvalContext &ctx, const SampledWavelengths &swl,
                                  const DataAccessor<float> *da) const noexcept = 0;
    [[nodiscard]] virtual Array<float> evaluate(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] Array<float> value(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept;
    void cache_value(const AttrEvalContext &ctx, const SampledWavelengths &swl, const DataAccessor<float> *da) const noexcept;
    void clear_cache() const noexcept;
    virtual void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};

class Slot : public ocarina::Hashable {
private:
    ShaderNode *_node{};
    uint _dim{4};
#ifndef NDEBUG
    string _channels;
#endif
    uint _channel_mask{};

private:
    [[nodiscard]] uint _calculate_mask(string channels) noexcept;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override;

public:
    Slot() = default;
    explicit Slot(ShaderNode *input, string channels)
        : _node(input),
          _dim(channels.size()),
#ifndef NDEBUG
          _channels(channels),
#endif
          _channel_mask(_calculate_mask(ocarina::move(channels))) {
        OC_ASSERT(_dim <= 4);
    }

    [[nodiscard]] uint dim() const noexcept { return _dim; }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] ColorDecode eval_albedo_spectrum(const AttrEvalContext &ctx,
                                                   const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] ColorDecode eval_unbound_spectrum(const AttrEvalContext &ctx,
                                                    const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] ColorDecode eval_illumination_spectrum(const AttrEvalContext &ctx,
                                                         const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] const ShaderNode *node() const noexcept { return _node; }
    [[nodiscard]] const ShaderNode *operator->() const noexcept { return _node; }
    [[nodiscard]] ShaderNode *node() noexcept { return _node; }
    [[nodiscard]] ShaderNode *operator->() noexcept { return _node; }
};

}// namespace vision