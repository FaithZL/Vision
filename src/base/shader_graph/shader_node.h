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

class ShaderGraph;
class ShaderNodeSlot;
#define VS_MAKE_SLOT(attr_name) ShaderNodeSlot attr_name##_{#attr_name};
#define VS_INIT_SLOT(name, default_value, type) \
    name##_.set(graph().construct_slot(desc, #name, default_value, type))
#define VS_INIT_SLOT_NO_DEFAULT(name, type) \
    name##_.set(graph().construct_slot(desc, #name, type))

class ShaderNodeSlotSet {
protected:
    struct SlotCursor {
        // The offset of the first slot in the object
        int offset{0u};
        uint num{0u};
    };
    SlotCursor slot_cursor_;

protected:
    ShaderNodeSlotSet() = default;
    void init_slot_cursor(const ShaderNodeSlot *ptr, uint num) noexcept;
    void init_slot_cursor(const ShaderNodeSlot *first, const ShaderNodeSlot *last) noexcept;
    const ShaderNodeSlot &get_slot(int index) const noexcept;
    ShaderNodeSlot &get_slot(int index) noexcept;
    template<bool check = true, typename T, typename F>
    auto reduce_slots(T &&initial, F &&func) const noexcept;
    template<bool check = true, typename T, typename F>
    auto reduce_slots(T &&initial, F &&func) noexcept;
    template<bool check = true, typename F>
    void for_each_slot(F &&func) const noexcept;
    template<bool check = true, typename F>
    void for_each_slot(F &&func) noexcept;
};

class ShaderNode : public Node, public Encodable, public enable_shared_from_this<ShaderNode> {
protected:
    weak_ptr<ShaderGraph> graph_;

public:
    using Desc = ShaderNodeDesc;
    static constexpr float s_cutoff = 1e-3f;

public:
    ShaderNode() = default;
    explicit ShaderNode(const ShaderNodeDesc &desc)
        : Node(desc) {}
    VS_HOTFIX_MAKE_RESTORE(Node, graph_)
    virtual bool render_UI_by_tag(Widgets *widgets, AttrTag attr_tag) noexcept {
        return render_UI(widgets);
    }
    virtual void initialize_slots(const ShaderNodeDesc &desc) noexcept;
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

    [[nodiscard]] virtual AttrEvalContext evaluate(const AttrEvalContext &ctx,
                                                   const SampledWavelengths &swl) const noexcept {
        OC_ERROR("call error");
        return {};
    }
    [[nodiscard]] virtual AttrEvalContext evaluate(const string &key, const AttrEvalContext &ctx,
                                                   const SampledWavelengths &swl) const noexcept {
        return evaluate(ctx, swl);
    }
    virtual void for_each_pixel(const function<Image::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};

class SlotsShaderNode : public ShaderNode, public ShaderNodeSlotSet {
public:
    SlotsShaderNode() = default;
    explicit SlotsShaderNode(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}

    void restore(vision::RuntimeObject *old_obj) noexcept override;

    ///#region encodable
    [[nodiscard]] uint compacted_size() const noexcept override;
    [[nodiscard]] bool has_device_value() const noexcept override;
    void after_decode() const noexcept override;
    void invalidate() const noexcept override;
    void encode(RegistrableManaged<buffer_ty> &data) const noexcept override;
    void decode(const DataAccessor *da) const noexcept override;
    void decode(const DynamicArray<ocarina::buffer_ty> &array) const noexcept override;
    [[nodiscard]] uint alignment() const noexcept override;
    [[nodiscard]] uint cal_offset(ocarina::uint prev_size) const noexcept override;
    ///#endregion

    ///#region GUI
    void reset_status() noexcept override;
    bool has_changed() noexcept override;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    ///#endregion
};

class SlotBase : public ocarina::Hashable {
protected:
    uint dim_{4};
    uint channel_mask_{};
#ifndef NDEBUG
    string channels_;
#endif
    string attr_name_{};
    AttrTag attr_tag_{};

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

class ShaderNodeSlot : public GUI, public Observer, public SlotBase {
protected:
    SP<ShaderNode> node_{};
    string output_key_;

protected:
    [[nodiscard]] uint64_t compute_hash() const noexcept override;
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override;

public:
    [[nodiscard]] static ShaderNodeSlot create_slot(const SlotDesc &desc);
    explicit ShaderNodeSlot(string attr_name = "") : SlotBase(std::move(attr_name)) {}
    ShaderNode &set(const ShaderNodeSlot &other) noexcept;
    explicit ShaderNodeSlot(SP<ShaderNode> input, string channels, AttrTag attr_tag, string key);
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    void reset_status() noexcept override;
    bool has_changed() noexcept override;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] virtual AttrEvalContext evaluate(const AttrEvalContext &ctx,
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

#define VS_MAKE_ENCODABLE_FUNC(RetType, func_name, custom_stmt) \
    template<typename... Args>                                  \
    RetType func_name(Args &&...args) const noexcept {          \
        if (node_) {                                            \
            return node_->func_name(OC_FORWARD(args)...);       \
        }                                                       \
        custom_stmt;                                            \
    }

    VS_MAKE_ENCODABLE_FUNC(decltype(auto), compacted_size, return 0u)
    VS_MAKE_ENCODABLE_FUNC(decltype(auto), encode, )
    VS_MAKE_ENCODABLE_FUNC(decltype(auto), update, )
    VS_MAKE_ENCODABLE_FUNC(decltype(auto), decode, )
    VS_MAKE_ENCODABLE_FUNC(decltype(auto), invalidate, )
    VS_MAKE_ENCODABLE_FUNC(decltype(auto), after_decode, )
    VS_MAKE_ENCODABLE_FUNC(decltype(auto), has_device_value, return true)
    VS_MAKE_ENCODABLE_FUNC(uint, cal_offset, return std::get<0>(std::tuple{OC_FORWARD(args)...}))
    VS_MAKE_ENCODABLE_FUNC(decltype(auto), alignment, return 0u)
#undef VS_MAKE_ENCODABLE_FUNC
};

template<bool check, typename T, typename F>
auto ShaderNodeSlotSet::reduce_slots(T &&initial, F &&func) const noexcept {
    T ret = OC_FORWARD(initial);
    for (int i = 0; i < slot_cursor_.num; ++i) {
        const ShaderNodeSlot &slot = get_slot(i);
        if constexpr (check) {
            if (slot) {
                ret = func(ret, slot);
            }
        } else {
            ret = func(ret, slot);
        }
    }
    return ret;
}

template<bool check, typename T, typename F>
auto ShaderNodeSlotSet::reduce_slots(T &&initial, F &&func) noexcept {
    T ret = OC_FORWARD(initial);
    for (int i = 0; i < slot_cursor_.num; ++i) {
        ShaderNodeSlot &slot = get_slot(i);
        if constexpr (check) {
            if (slot) {
                ret = func(ret, slot);
            }
        } else {
            ret = func(ret, slot);
        }
    }
    return ret;
}

template<bool check, typename F>
void ShaderNodeSlotSet::for_each_slot(F &&func) const noexcept {
    for (int i = 0; i < slot_cursor_.num; ++i) {
        const ShaderNodeSlot &slot = get_slot(i);
        if constexpr (check) {
            if (slot) {
                func(slot);
            }
        } else {
            func(slot);
        }
    }
}

template<bool check, typename F>
void ShaderNodeSlotSet::for_each_slot(F &&func) noexcept {
    for (int i = 0; i < slot_cursor_.num; ++i) {
        ShaderNodeSlot &slot = get_slot(i);
        if constexpr (check) {
            if (slot) {
                func(slot);
            }
        } else {
            func(slot);
        }
    }
}

}// namespace vision