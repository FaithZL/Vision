//
// Created by Zero on 23/01/2023.
//

#include "shader_node.h"

#include <utility>
#include "base/mgr/pipeline.h"
#include "GUI/widgets.h"

namespace vision {

///#region SlotBase
SlotBase::SlotBase(int, std::string channels, AttrTag attr_tag)
    : dim_(channels.size()),
#ifndef NDEBUG
      channels_(channels),
#endif
      channel_mask_(calculate_mask(ocarina::move(channels))),
      attr_tag_(attr_tag) {
}

uint64_t SlotBase::compute_hash() const noexcept {
    return hash64(channel_mask_, dim_, node()->hash(), attr_tag_);
}

uint64_t SlotBase::compute_topology_hash() const noexcept {
    return hash64(channel_mask_, dim_, node()->topology_hash(), attr_tag_);
}

uint SlotBase::calculate_mask(string channels) noexcept {
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
///#endregion

///#region OutputSlot
OutputSlot::OutputSlot(const vision::InputSlot &slot)
    : SlotBase(static_cast<const SlotBase &>(slot)), node_(slot.node_) {}
///#endregion

///#region Slot
InputSlot::InputSlot(SP<vision::ShaderNode> input, std::string channels, AttrTag attr_tag)
    : SlotBase(0, std::move(channels), attr_tag), node_(std::move(input)) {
    OC_ASSERT(dim_ <= 4);
}

InputSlot InputSlot::create_slot(const vision::SlotDesc &desc) {
    SP<ShaderNode> shader_node = Node::create_shared<ShaderNode>(desc.node);
    return InputSlot(shader_node, desc.channels, desc.attr_tag);
}

ShaderNode &InputSlot::set(const vision::InputSlot &other) noexcept {
    string old_name = attr_name_;
    *this = other;
    if (other.attr_name_.empty()) {
        attr_name_ = old_name;
    }
    return *node_;
}

void InputSlot::update_runtime_object(const vision::IObjectConstructor *constructor) noexcept {
    if (node_) {
        HotfixSystem::replace_objects(constructor, std::tuple{addressof(node_)});
    }
}

void InputSlot::reset_status() noexcept {
    if (node_) {
        node_->reset_status();
    }
}

bool InputSlot::has_changed() noexcept {
    if (node_) {
        return node_->has_changed();
    }
    return false;
}

void InputSlot::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    if (node_) {
        node_->render_sub_UI(widgets);
    }
}

bool InputSlot::render_UI(ocarina::Widgets *widgets) noexcept {
    if (node_) {
        if (!attr_name_.empty()) {
            node_->set_name(attr_name_);
        }
        return node_->render_UI_by_tag(widgets, attr_tag_);
    }
    return false;
}

DynamicArray<float> InputSlot::evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept {
    switch (dim_) {
        case 1: {
            switch (channel_mask_) {
#include "slot_swizzle_1.inl.h"
            }
        }
        case 2: {
            switch (channel_mask_) {
#include "slot_swizzle_2.inl.h"
            }
        }
        case 3: {
            switch (channel_mask_) {
#include "slot_swizzle_3.inl.h"
            }
        }
        case 4: {
            switch (channel_mask_) {
#include "slot_swizzle_4.inl.h"
            }
        }
    }
    return node_->evaluate(ctx, swl);
}

vector<float> InputSlot::average() const noexcept {
    switch (dim_) {
        case 1: {
            switch (channel_mask_) {
#include "slot_average_swizzle_1.inl.h"
            }
        }
        case 2: {
            switch (channel_mask_) {
#include "slot_average_swizzle_2.inl.h"
            }
        }
        case 3: {
            switch (channel_mask_) {
#include "slot_average_swizzle_3.inl.h"
            }
        }
        case 4: {
            switch (channel_mask_) {
#include "slot_average_swizzle_4.inl.h"
            }
        }
    }
    OC_ASSERT(false);
    return {};
}

float InputSlot::luminance() const noexcept {
    switch (dim_) {
        case 1: return average()[0];
        case 2:
            OC_ERROR("float2 not is a color attribute !");
            return 0;
        case 3:
        case 4:
            auto a = average();
            return ocarina::luminance(make_float3(a[0], a[1], a[2]));
    }
    OC_ASSERT(false);
    return 0;
}

ColorDecode InputSlot::eval_albedo_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(dim_ == 3);
    Float3 val = evaluate(ctx, swl).as_vec3();
    return node_->spectrum()->decode_to_albedo(val, swl);
}

ColorDecode InputSlot::eval_unbound_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(dim_ == 3);
    Float3 val = evaluate(ctx, swl).as_vec3();
    return node_->spectrum()->decode_to_unbound_spectrum(val, swl);
}

ColorDecode InputSlot::eval_illumination_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(dim_ == 3);
    Float3 val = evaluate(ctx, swl).as_vec3();
    return node_->spectrum()->decode_to_illumination(val, swl);
}
///#endregion

ShaderNode &ShaderNode::set_graph(const SP<ShaderGraph> &graph) noexcept {
    graph_ = graph;
    return *this;
}

ShaderGraph &ShaderNode::graph() const noexcept {
    return *graph_.lock().get();
}

ShaderNode &ShaderNode::add_to(ShaderGraph &graph) noexcept {
    return add_to(name(), graph);
}

ShaderNode &ShaderNode::add_to(const std::string &name, vision::ShaderGraph &graph) noexcept {
    graph.add_node(name, shared_from_this());
    return *this;
}

uint64_t RootSlot::compute_hash() const noexcept {
    return hash64(InputSlot::compute_hash(), src_node_->hash(), key_);
}

uint64_t RootSlot::compute_topology_hash() const noexcept {
    return hash64(InputSlot::compute_topology_hash(), src_node_->topology_hash(), key_);
}

#define VS_ROOT_SLOT_FUNC_IMPL(Ret, func_name)                                  \
    Ret RootSlot::func_name(const AttrEvalContext &ctx,                         \
                            const SampledWavelengths &swl) const noexcept {     \
        if (src_node_) {                                                        \
            return InputSlot::func_name(src_node_->apply(ctx, swl, key_), swl); \
        }                                                                       \
        return InputSlot::func_name(ctx, swl);                                  \
    }

VS_ROOT_SLOT_FUNC_IMPL(float_array, evaluate)
VS_ROOT_SLOT_FUNC_IMPL(ColorDecode, eval_albedo_spectrum)
VS_ROOT_SLOT_FUNC_IMPL(ColorDecode, eval_unbound_spectrum)
VS_ROOT_SLOT_FUNC_IMPL(ColorDecode, eval_illumination_spectrum)

#undef VS_ROOT_SLOT_FUNC_IMPL

}// namespace vision