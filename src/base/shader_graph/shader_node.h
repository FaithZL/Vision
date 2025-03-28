//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/node.h"
#include "util/image.h"
#include "base/color/spectrum.h"
#include "base/scattering/interaction.h"
#include "hotfix/hotfix.h"

namespace vision {

class ShaderNode;
#define VS_MAKE_SLOT(attr_name) Slot attr_name##_{#attr_name};
#define INIT_SLOT(name, default_value, type) \
    name##_.set(Slot::create_slot(desc.slot(#name, default_value, type)))

class Slot : public ocarina::Hashable, public GUI, public Observer {
private:
    SP<ShaderNode> node_{};
    uint dim_{4};
#ifndef NDEBUG
    string channels_;
#endif
    uint channel_mask_{};
    string attr_name_{};

private:
    [[nodiscard]] static uint calculate_mask(string channels) noexcept;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override;

public:
    [[nodiscard]] static Slot create_slot(const SlotDesc &desc);

    explicit Slot(string attr_name = "") : attr_name_(std::move(attr_name)) {}
    Slot &set(const Slot &other) noexcept;
    explicit Slot(SP<ShaderNode> input, string channels);
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    void reset_status() noexcept override;
    bool has_changed() noexcept override;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] uint dim() const noexcept { return dim_; }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                               const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] vector<float> average() const noexcept;
    [[nodiscard]] float luminance() const noexcept;
    [[nodiscard]] bool valid() const noexcept { return node_ != nullptr; }
    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    [[nodiscard]] ColorDecode eval_albedo_spectrum(const AttrEvalContext &ctx,
                                                   const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] ColorDecode eval_unbound_spectrum(const AttrEvalContext &ctx,
                                                    const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] ColorDecode eval_illumination_spectrum(const AttrEvalContext &ctx,
                                                         const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] const ShaderNode *node() const noexcept { return node_.get(); }
    [[nodiscard]] const ShaderNode *operator->() const noexcept { return node_.get(); }
    [[nodiscard]] ShaderNode *node() noexcept { return node_.get(); }
    [[nodiscard]] ShaderNode *operator->() noexcept { return node_.get(); }
};

class ShaderNode : public Node, public Encodable, public enable_shared_from_this<ShaderNode> {
protected:
    ShaderNodeTag node_tag_{};
    vector<weak_ptr<Node>> outputs_{};

public:
    using Desc = ShaderNodeDesc;
    static constexpr float s_cutoff = 1e-3f;

public:
    ShaderNode() = default;
    explicit ShaderNode(const ShaderNodeDesc &desc) : Node(desc), node_tag_(desc.node_tag) {}
    VS_HOTFIX_MAKE_RESTORE(Node, node_tag_)
    [[nodiscard]] virtual uint dim() const noexcept { return 4; }
    OC_MAKE_MEMBER_GETTER(node_tag, )
    [[nodiscard]] virtual bool near_zero() const noexcept { return false; }
    [[nodiscard]] virtual bool near_one() const noexcept { return false; }
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

    ///#region for NumberInput
    virtual void set_range(float lower, float upper) noexcept {}
    virtual void update_value(vector<float> values) noexcept {}
    virtual float normalize() noexcept { return 1.f; }
    ///#endregion

    [[nodiscard]] virtual DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                                       const SampledWavelengths &swl) const noexcept = 0;
    virtual void for_each_pixel(const function<Image::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};
}// namespace vision