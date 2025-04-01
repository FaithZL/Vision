//
// Created by Zero on 2025/3/28.
//

#pragma once

#include "shader_node.h"

namespace vision {

class ShaderGraph : public enable_shared_from_this<ShaderGraph> {
protected:
    map<string, SP<ShaderNode>> nodes_;

public:
    void add_node(SP<ShaderNode> node) noexcept {
        add_node(node->name(), node);
    }
    void add_node(const string &name, SP<ShaderNode> node) {
        if (nodes_.contains(name)) {
            return;
        }
        node->set_graph(shared_from_this());
        nodes_.insert(make_pair(name, node));
    }
    void clear() noexcept {
        for (auto &it : nodes_) {
            it.second->set_graph(nullptr);
        }
        nodes_.clear();
    }
    [[nodiscard]] ShaderGraph &graph() noexcept { return *this; }
    [[nodiscard]] SP<ShaderNode> get_node(const string &name) const noexcept {
        return nodes_.at(name);
    }
};

}// namespace vision