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

}// namespace vision