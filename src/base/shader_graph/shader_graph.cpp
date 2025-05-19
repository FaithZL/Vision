//
// Created by Zero on 2025/4/9.
//

#include "shader_graph.h"

namespace vision {

void ShaderGraph::add_node(const std::string &name, SP<vision::ShaderNode> node) {
    if (node_map_.contains(name)) {
        return;
    }
    node->set_graph(shared_from_this());
    node_map_.insert(make_pair(name, node));
}

void ShaderGraph::clear() noexcept {
    for (auto &it : node_map_) {
        it.second->set_graph(nullptr);
    }
    node_map_.clear();
}

void ShaderGraph::init_node_map(const map<string, ShaderNodeDesc> &tab) noexcept {
    for (const auto &[key, desc] : tab) {
        auto shader_node = Node::create_shared<ShaderNode>(desc);
        add_node(key, shader_node);
        shader_node->initialize_slots(desc);
    }
}

ShaderNodeSlot ShaderGraph::construct_slot(const vision::ParameterSet &ps, vision::AttrTag tag) const noexcept {
    DataWrap data = ps.data();
    if (data.is_null()) {
        return ShaderNodeSlot{};
    }
    string str = to_string(data);
    SP<ShaderNode> shader_node = get_node(data["node"]);
    ShaderNodeSlot slot = ShaderNodeSlot(shader_node, data["channels"], tag,
                                         ps.value("output_key").as_string());
    return slot;
}

template<typename T>
[[nodiscard]] ShaderNodeSlot ShaderGraph::construct_slot(const AttrDesc &desc, const string &attr_name,
                                                         T val, AttrTag tag) noexcept {
    ParameterSet ps = desc.value(attr_name);
    DataWrap data = ps.data();
    string str = to_string(data);
    ShaderNodeSlot slot;
    if (data.contains("node") && data["node"].is_string()) {
        slot = construct_slot(ps, tag);
    } else {
        SlotDesc slot_desc = desc.slot(attr_name, val, tag);
        slot = ShaderNodeSlot::create_slot(slot_desc);
        slot->set_graph(shared_from_this());
    }
    return slot;
}

ShaderNodeSlot ShaderGraph::construct_slot(const AttrDesc &desc, const string &attr_name,
                                           vision::AttrTag tag) const noexcept {
    ParameterSet ps = desc.value(attr_name);
    return construct_slot(ps, tag);
}

#define VS_INSTANCE_CONSTRUCT_SLOT(type)                                                                     \
    template ShaderNodeSlot ShaderGraph::construct_slot<type>(const AttrDesc &desc, const string &attr_name, \
                                                              type val, AttrTag tag) noexcept;

VS_INSTANCE_CONSTRUCT_SLOT(float)
VS_INSTANCE_CONSTRUCT_SLOT(float2)
VS_INSTANCE_CONSTRUCT_SLOT(float3)
VS_INSTANCE_CONSTRUCT_SLOT(float4)

#undef VS_INSTANCE_CONSTRUCT_SLOT

}// namespace vision