//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/node.h"
#include "util/file_manager.h"
#include "base/shader_graph/shader_node.h"

namespace vision {

class Light;
class ShaderNode;

class NodeMgr {
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
    template<typename T, typename desc_ty>
    [[nodiscard]] SP<T> load(const desc_ty &desc) {
        const DynamicModule *module = FileManager::instance().obtain_module(desc.plugin_name());
        auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
        auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
        SP<T> ret = SP<T>(dynamic_cast<T *>(creator(desc)), deleter);
        OC_ERROR_IF(ret == nullptr, "error node load ", desc.name);
        return ret;
    }
    [[nodiscard]] SP<ShaderNode> load_shader_node(const ShaderNodeDesc &desc);
    [[nodiscard]] Slot create_slot(const SlotDesc &desc);
};
}// namespace vision
