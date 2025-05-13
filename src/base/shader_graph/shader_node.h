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
class ShaderGraph;
#define VS_MAKE_SLOT(attr_name) InputSlot attr_name##_{#attr_name};
#define INIT_SLOT(name, default_value, type) \
    name##_.set(graph().construct_slot(desc, #name, default_value, type))

class SlotBase : public ocarina::Hashable {
protected:
    uint dim_{4};
    uint channel_mask_{};
#ifndef NDEBUG
    string channels_;
#endif
    string attr_name_{};
    AttrTag attr_tag_{};

protected:
    [[nodiscard]] uint64_t compute_hash() const noexcept override;
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override;

public:
    explicit SlotBase(string attr_name = "") : attr_name_(std::move(attr_name)) {}
    SlotBase(int, string channels, AttrTag attr_tag);
    [[nodiscard]] static uint calculate_mask(string channels) noexcept;
    OC_MAKE_MEMBER_GETTER(dim, )
    OC_MAKE_MEMBER_GETTER(channel_mask, )
    OC_MAKE_MEMBER_GETTER(attr_name, )
    OC_MAKE_MEMBER_GETTER_SETTER(attr_tag, )
    [[nodiscard]] virtual const ShaderNode *node() const noexcept = 0;
    [[nodiscard]] virtual ShaderNode *node() noexcept = 0;
    [[nodiscard]] const ShaderNode *operator->() const noexcept { return node(); }
    [[nodiscard]] ShaderNode *operator->() noexcept { return node(); }
};

class OutputSlot;

class InputSlot : public GUI, public Observer, public SlotBase {
protected:
    SP<ShaderNode> node_{};
    string output_key_;
    friend class OutputSlot;

public:
    [[nodiscard]] static InputSlot create_slot(const SlotDesc &desc);
    explicit InputSlot(string attr_name = "") : SlotBase(std::move(attr_name)) {}
    ShaderNode &set(const InputSlot &other) noexcept;
    explicit InputSlot(SP<ShaderNode> input, string channels, AttrTag attr_tag, string key = "");
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    void reset_status() noexcept override;
    bool has_changed() noexcept override;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] virtual DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                                       const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] vector<float> average() const noexcept;
    [[nodiscard]] float luminance() const noexcept;
    [[nodiscard]] bool valid() const noexcept { return node_ != nullptr; }
    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    [[nodiscard]] virtual ColorDecode eval_albedo_spectrum(const AttrEvalContext &ctx,
                                                           const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_unbound_spectrum(const AttrEvalContext &ctx,
                                                            const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_illumination_spectrum(const AttrEvalContext &ctx,
                                                                 const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] const ShaderNode *node() const noexcept override { return node_.get(); }
    [[nodiscard]] ShaderNode *node() noexcept override { return node_.get(); }
};

class OutputSlot : public SlotBase {
private:
    weak_ptr<ShaderNode> node_{};
    string key_;

public:
    explicit OutputSlot(const InputSlot &slot);
    [[nodiscard]] const ShaderNode *node() const noexcept override { return node_.lock().get(); }
    [[nodiscard]] ShaderNode *node() noexcept override { return node_.lock().get(); }
};

class ShaderNode : public Node, public Encodable, public enable_shared_from_this<ShaderNode> {
protected:
    vector<OutputSlot> outputs_{};
    weak_ptr<ShaderGraph> graph_;

public:
    using Desc = ShaderNodeDesc;
    static constexpr float s_cutoff = 1e-3f;

public:
    ShaderNode() = default;
    explicit ShaderNode(const ShaderNodeDesc &desc)
        : Node(desc) {}
    ShaderNode &add_output(const InputSlot &slot) noexcept {
        outputs_.emplace_back(slot);
        return *this;
    }
    VS_HOTFIX_MAKE_RESTORE(Node, outputs_, graph_)
    virtual bool render_UI_by_tag(Widgets *widgets, AttrTag attr_tag) noexcept {
        return render_UI(widgets);
    }
    ShaderNode &set_graph(const SP<ShaderGraph> &graph) noexcept;
    [[nodiscard]] ShaderGraph &graph() const noexcept;
    [[nodiscard]] virtual uint dim() const noexcept { return 4; }
    [[nodiscard]] virtual bool near_zero() const noexcept { return false; }
    [[nodiscard]] virtual bool near_one() const noexcept { return false; }
    ShaderNode &add_to(ShaderGraph &graph) noexcept;
    ShaderNode &add_to(const string &name, ShaderGraph &graph) noexcept;
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
    [[nodiscard]] virtual ocarina::vector<float> average() const noexcept {
        OC_ERROR("call error");
        return {};
    }

    ///#region for NumberArray
    virtual ShaderNode &set_range(float lower, float upper) noexcept { return *this; }
    virtual ShaderNode &update_value(vector<float> values) noexcept { return *this; }
    virtual float normalize() noexcept { return 1.f; }
    ///#endregion

    [[nodiscard]] virtual DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                                       const SampledWavelengths &swl) const noexcept {
        OC_ERROR("call error");
        return {};
    }
    [[nodiscard]] virtual float_array evaluate(const string &key, const AttrEvalContext &ctx,
                                               const SampledWavelengths &swl) const noexcept {
        return evaluate(ctx, swl);
    }
    virtual void for_each_pixel(const function<Image::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};

class SourceNode : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    [[nodiscard]] virtual AttrEvalContext apply(const AttrEvalContext &ctx,
                                                const SampledWavelengths &swl,
                                                const string &key) const noexcept {
        return ctx;
    }
};

/**
 * Root Slot for material or light's attribute
 */
class RootSlot : public InputSlot {
private:
    SP<SourceNode> src_node_;
    string src_key_;

protected:
    [[nodiscard]] uint64_t compute_hash() const noexcept override;
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override;

public:
    using InputSlot::InputSlot;
    [[nodiscard]] float_array evaluate(const AttrEvalContext &ctx,
                                       const SampledWavelengths &swl) const noexcept override;
    [[nodiscard]] ColorDecode eval_albedo_spectrum(const AttrEvalContext &ctx,
                                                   const SampledWavelengths &swl) const noexcept override;
    [[nodiscard]] ColorDecode eval_unbound_spectrum(const AttrEvalContext &ctx,
                                                    const SampledWavelengths &swl) const noexcept override;
    [[nodiscard]] ColorDecode eval_illumination_spectrum(const AttrEvalContext &ctx,
                                                         const SampledWavelengths &swl) const noexcept override;
};

}// namespace vision