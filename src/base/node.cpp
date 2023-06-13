//
// Created by Zero on 16/01/2023.
//

#include "mgr/pipeline.h"
#include "node.h"

namespace vision {

Pipeline *Node::pipeline() noexcept {
    return _scene->pipeline();
}

const Pipeline *Node::pipeline() const noexcept {
    return _scene->pipeline();
}

Spectrum &Node::spectrum() noexcept {
    return pipeline()->spectrum();
}

const Spectrum &Node::spectrum() const noexcept {
    return pipeline()->spectrum();
}

Device &Node::device() noexcept {
    return pipeline()->device();
}

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

NodeMgr::Iterator NodeMgr::remove(vision::Node *node) noexcept {
    return std::remove_if(_all_nodes.begin(), _all_nodes.end(), [&](Node::Wrapper &elm) {
        return elm.get() == node;
    });
}

}// namespace vision