//
// Created by Zero on 2025/3/28.
//

#pragma once

#include "shader_node.h"

namespace vision {

class ShaderGraph : public enable_shared_from_this<ShaderGraph> {
protected:
    bool is_root_{false};
    weak_ptr<ShaderGraph> graph_;
    map<string, SP<ShaderNode>> node_map_;

public:
    void add_node(SP<ShaderNode> node) noexcept {
        add_node(node->name(), node);
    }
    void add_node(const string &name, SP<ShaderNode> node);
    void clear() noexcept;
    void init_node_map(const map<string, ShaderNodeDesc> &tab) noexcept;
    [[nodiscard]] ShaderNodeSlot construct_slot(const ParameterSet &ps, AttrTag tag) const noexcept;
    [[nodiscard]] ShaderNodeSlot construct_slot(const AttrDesc &desc, const string &attr_name,
                                                AttrTag tag) const noexcept;
    template<typename T>
    [[nodiscard]] ShaderNodeSlot construct_slot(const AttrDesc &desc, const string &attr_name,
                                                T val, AttrTag tag) noexcept;
    [[nodiscard]] ShaderGraph &graph() noexcept {
        if (is_root_) {
            return *this;
        }
        return *graph_.lock();
    }
    [[nodiscard]] SP<const ShaderGraph> shared_graph() const noexcept {
        if (is_root_) {
            return shared_from_this();
        }
        return graph_.lock();
    }
    [[nodiscard]] SP<ShaderGraph> shared_graph() noexcept {
        if (is_root_) {
            return shared_from_this();
        }
        return graph_.lock();
    }
    OC_MAKE_MEMBER_GETTER_SETTER(is_root,)
    void set_graph(SP<ShaderGraph> graph) noexcept {
        OC_ASSERT(!is_root_);
        graph_ = graph;
    }
    [[nodiscard]] SP<ShaderNode> get_node(const string &name) const noexcept {
        return node_map_.at(name);
    }
};

}// namespace vision