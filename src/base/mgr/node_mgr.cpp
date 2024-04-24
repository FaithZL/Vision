//
// Created by Zero on 2023/6/14.
//

#include "node_mgr.h"
#include "base/node.h"
#include "pipeline.h"

namespace vision {

NodeMgr *NodeMgr::s_node_loader = nullptr;

NodeMgr &NodeMgr::instance() noexcept {
    if (s_node_loader == nullptr) {
        s_node_loader = new vision::NodeMgr();
    }
    return *s_node_loader;
}

void NodeMgr::destroy_instance() noexcept {
    if (s_node_loader) {
        delete s_node_loader;
        s_node_loader = nullptr;
    }
}

Slot NodeMgr::create_slot(const SlotDesc &desc) {
    SP<ShaderNode> shader_node = Node::load<ShaderNode>(desc.node);
    return Slot(shader_node, desc.channels);
}
}// namespace vision