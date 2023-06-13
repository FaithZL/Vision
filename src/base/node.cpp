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

NodeMgr::Iterator NodeMgr::remove(vision::Node *node) {
    return std::remove_if(_all_nodes.begin(), _all_nodes.end(), [&](Node::Wrapper &elm) {
        return elm.get() == node;
    });
}

//Node *NodeMgr::load_node(const vision::NodeDesc &desc) {
//    const DynamicModule *module = _context->obtain_module(desc.plugin_name());
//    auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
//    auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
//    _all_nodes.emplace_back(creator(desc), deleter);
//    return _all_nodes.back().get();
//}

}// namespace vision