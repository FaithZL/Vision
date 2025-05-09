//
// Created by Zero on 2025/3/28.
//

#pragma once

#include "shader_node.h"

namespace vision {

class ShaderGraph : public enable_shared_from_this<ShaderGraph> {
protected:
    map<string, SP<ShaderNode>> node_map_;

public:
    void add_node(SP<ShaderNode> node) noexcept {
        add_node(node->name(), node);
    }
    void add_node(const string &name, SP<ShaderNode> node);
    void clear() noexcept;
    void init_node_map(const map<string, ShaderNodeDesc> &tab) noexcept;
    template<typename T>
    [[nodiscard]] InputSlot construct_slot(const MaterialDesc &desc, const string &attr_name,
                                      T val, AttrTag tag) noexcept;
    [[nodiscard]] ShaderGraph &graph() noexcept { return *this; }
    [[nodiscard]] SP<ShaderNode> get_node(const string &name) const noexcept {
        return node_map_.at(name);
    }
};

}// namespace vision