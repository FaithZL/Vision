//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/node.h"
#include "util/image_io.h"
#include "base/color/spectrum.h"
#include "base/scattering/interaction.h"

namespace vision {

class ShaderNode : public Node, public Serializable<float> {
protected:
    ShaderNodeType _type{};

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
    [[nodiscard]] virtual ocarina::vector<float> average() const noexcept = 0;
    [[nodiscard]] virtual DynamicArray<float> evaluate(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept = 0;
    virtual void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};

class Slot : public ocarina::Hashable, public GUI {
private:
    SP<ShaderNode> _node{};
    string _attr_name{};
    uint _dim{4};
#ifndef NDEBUG
    string _channels;
#endif
    uint _channel_mask{};

private:
    [[nodiscard]] static uint _calculate_mask(string channels) noexcept;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override;

public:
    explicit Slot(string name = "") : _attr_name(name) {}
    explicit Slot(SP<ShaderNode> input, string channels, string name = "")
        : _node(input),
          _dim(channels.size()),
#ifndef NDEBUG
          _channels(channels),
#endif
          _channel_mask(_calculate_mask(ocarina::move(channels))),
          _attr_name(name) {
        OC_ASSERT(_dim <= 4);
    }

    void reset_status() noexcept override {
        if (_node) {
            _node->reset_status();
        }
    }

    bool has_changed() noexcept override {
        if (_node) {
            return _node->has_changed();
        }
        return false;
    }

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        if (_node) {
            return _node->render_UI(widgets);
        }
        return false;
    }

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        if (_node) {
            _node->render_sub_UI(widgets);
        }
    }

    [[nodiscard]] uint dim() const noexcept { return _dim; }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                               const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] vector<float> average() const noexcept;
    [[nodiscard]] float luminance() const noexcept;
    [[nodiscard]] bool valid() const noexcept { return _node != nullptr; }
    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    [[nodiscard]] ColorDecode eval_albedo_spectrum(const AttrEvalContext &ctx,
                                                   const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] ColorDecode eval_unbound_spectrum(const AttrEvalContext &ctx,
                                                    const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] ColorDecode eval_illumination_spectrum(const AttrEvalContext &ctx,
                                                         const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] const ShaderNode *node() const noexcept { return _node.get(); }
    [[nodiscard]] const ShaderNode *operator->() const noexcept { return _node.get(); }
    [[nodiscard]] ShaderNode *node() noexcept { return _node.get(); }
    [[nodiscard]] ShaderNode *operator->() noexcept { return _node.get(); }
};

}// namespace vision