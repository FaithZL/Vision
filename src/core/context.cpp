//
// Created by Zero on 05/10/2022.
//

#include "context.h"
#include "descriptions/scene_desc.h"

namespace vision {

Context::Context(int argc, char **argv, ocarina::string_view cache_dir)
    : Super(fs::path(argv[0]).parent_path(), cache_dir),
      _cli_parser(argc, argv) {
}

SceneDesc Context::parse_file() const noexcept {
    return SceneDesc::from_json(cli_parser().scene_file());
}

Node* Context::load_node(const NodeDesc *desc) {
    const DynamicModule *module = obtain_module(desc->plugin_name());
    auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
    auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
    _all_nodes.emplace_back(creator(desc), deleter);
    return _all_nodes.back().get();
}

}// namespace vision