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

///#region Slot
ShaderNodeSlot::ShaderNodeSlot(SP<vision::ShaderNode> input, std::string channels, AttrTag attr_tag, string key)
    : SlotBase(0, std::move(channels), attr_tag),
      node_(std::move(input)),
      output_key_(std::move(key)) {
    OC_ASSERT(dim_ <= 4);
}

uint64_t ShaderNodeSlot::compute_hash() const noexcept {
    return hash64(channel_mask_, dim_, (node_ ? node_->hash() : 0u), attr_tag_);
}

uint64_t ShaderNodeSlot::compute_topology_hash() const noexcept {
    return hash64(channel_mask_, dim_, (node_ ? node_->topology_hash() : 0u), attr_tag_);
}

ShaderNodeSlot ShaderNodeSlot::create_slot(const vision::SlotDesc &desc) {
    SP<ShaderNode> shader_node = Node::create_shared<ShaderNode>(desc.node);
    return ShaderNodeSlot(shader_node, desc.channels, desc.attr_tag, desc.output_key);
}

ShaderNode &ShaderNodeSlot::set(const vision::ShaderNodeSlot &other) noexcept {
    string old_name = attr_name_;
    *this = other;
    if (other.attr_name_.empty()) {
        attr_name_ = old_name;
    }
    reset_topology_hash();
    reset_hash();
    return *node_;
}

void ShaderNodeSlot::update_runtime_object(const vision::IObjectConstructor *constructor) noexcept {
    if (node_) {
        HotfixSystem::replace_objects(constructor, std::tuple{addressof(node_)});
    }
}

void ShaderNodeSlot::reset_status() noexcept {
    if (node_) {
        node_->reset_status();
    }
}

bool ShaderNodeSlot::has_changed() noexcept {
    if (node_) {
        return node_->has_changed();
    }
    return false;
}

void ShaderNodeSlot::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    if (node_) {
        node_->render_sub_UI(widgets);
    }
}

bool ShaderNodeSlot::render_UI(ocarina::Widgets *widgets) noexcept {
    if (node_) {
        if (!attr_name_.empty()) {
            node_->set_name(attr_name_);
        }
        return node_->render_UI_by_tag(widgets, attr_tag_);
    }
    return false;
}

AttrEvalContext ShaderNodeSlot::evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept {
    if (!node_) {
        return ctx;
    }
    if (!output_key_.empty()) {
        return node_->evaluate(output_key_, ctx, swl);
    }
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

vector<float> ShaderNodeSlot::average() const noexcept {
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

float ShaderNodeSlot::luminance() const noexcept {
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

ColorDecode ShaderNodeSlot::eval_albedo_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(dim_ == 3);
    Float3 val = evaluate(ctx, swl)->as_vec3();
    return node_->spectrum()->decode_to_albedo(val, swl);
}

ColorDecode ShaderNodeSlot::eval_unbound_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(dim_ == 3);
    Float3 val = evaluate(ctx, swl)->as_vec3();
    return node_->spectrum()->decode_to_unbound_spectrum(val, swl);
}

ColorDecode ShaderNodeSlot::eval_illumination_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(dim_ == 3);
    Float3 val = evaluate(ctx, swl)->as_vec3();
    return node_->spectrum()->decode_to_illumination(val, swl);
}
///#endregion

void ShaderNode::initialize_slots(const vision::ShaderNodeDesc &desc) noexcept {
}

ShaderNode &ShaderNode::set_graph(const SP<ShaderGraph> &graph) noexcept {
    graph_ = graph;
    return *this;
}

ShaderGraph &ShaderNode::graph() const noexcept {
    return *graph_.lock();
}

ShaderNode &ShaderNode::add_to(ShaderGraph &graph) noexcept {
    return add_to(name(), graph);
}

ShaderNode &ShaderNode::add_to(const std::string &name, vision::ShaderGraph &graph) noexcept {
    graph.add_node(name, shared_from_this());
    return *this;
}

}// namespace vision