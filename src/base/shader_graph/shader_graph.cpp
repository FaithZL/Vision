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
        node_map_.insert(make_pair(key, shader_node));
    }
}

template<typename T>
[[nodiscard]] InputSlot ShaderGraph::construct_slot(const MaterialDesc &desc, const string &attr_name,
                                                    T val, AttrTag tag) noexcept {
    ParameterSet ps = desc.value(attr_name);
    DataWrap data = ps.data();
    string str = to_string(data);
    InputSlot slot;
    if (data.contains("node") && data["node"].is_string()) {
        SP<ShaderNode> shader_node = get_node(data["node"]);
        slot = InputSlot(shader_node, data["channels"],
                         tag, ps.value("output_key").as_string());
    } else {
        SlotDesc slot_desc = desc.slot(attr_name, val, tag);
        slot = InputSlot::create_slot(slot_desc);
    }
    slot->set_graph(shared_from_this());
    return slot;
}

#define VS_INSTANCE_CONSTRUCT_SLOT(type)                                                                    \
    template InputSlot ShaderGraph::construct_slot<type>(const MaterialDesc &desc, const string &attr_name, \
                                                         type val, AttrTag tag) noexcept;

VS_INSTANCE_CONSTRUCT_SLOT(float)
VS_INSTANCE_CONSTRUCT_SLOT(float2)
VS_INSTANCE_CONSTRUCT_SLOT(float3)
VS_INSTANCE_CONSTRUCT_SLOT(float4)

#undef VS_INSTANCE_CONSTRUCT_SLOT

}// namespace vision