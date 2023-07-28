//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/node.h"
#include "base/shader_graph/shader_node.h"

namespace vision {

class Light;
class ShaderNode;

class NodeMgr {
public:
    using Container = std::list<Node::Wrapper>;
    using Iterator = Container::iterator;

private:
    NodeMgr() = default;
    NodeMgr(const NodeMgr &) = delete;
    NodeMgr(NodeMgr &&) = delete;
    NodeMgr operator=(const NodeMgr &) = delete;
    NodeMgr operator=(NodeMgr &&) = delete;
    static NodeMgr *s_node_loader;

public:
    [[nodiscard]] static NodeMgr &instance() noexcept;
    static void destroy_instance() noexcept;
    [[nodiscard]] SP<Node> load_node(const NodeDesc &desc);
    template<typename T, typename desc_ty>
    [[nodiscard]] SP<T> load(const desc_ty &desc) {
        SP<T> ret = std::dynamic_pointer_cast<T>(load_node(desc));
        OC_ERROR_IF(ret == nullptr, "error node load ", desc.name);
        return ret;
    }
    [[nodiscard]] SP<ShaderNode> load_shader_node(const ShaderNodeDesc &desc);
    [[nodiscard]] Slot create_slot(const SlotDesc &desc);
};
}// namespace vision
