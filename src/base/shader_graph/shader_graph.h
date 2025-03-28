//
// Created by Zero on 2025/3/28.
//

#pragma once

#include "shader_node.h"

namespace vision {

class ShaderGraph : public enable_shared_from_this<ShaderGraph> {
protected:
    vector<SP<ShaderNode>> nodes_;

public:
    void add_node(SP<ShaderNode> node) noexcept {
        node->set_graph(shared_from_this());
        nodes_.push_back(std::move(node));
    }
    void clear() noexcept {
        for (SP<ShaderNode> node : nodes_) {
            node->set_graph(nullptr);
        }
        nodes_.clear();
    }
};

}// namespace vision