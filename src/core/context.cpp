//
// Created by Zero on 05/10/2022.
//

#include "context.h"

namespace vision {

Context::Context(int argc, char **argv, ocarina::string_view cache_dir)
    : Super(fs::path(argv[0]).parent_path(), cache_dir),
      _cli_parser(argc, argv) {
}

Node::Handle Context::load_plugin(NodeDesc *desc) {
    const DynamicModule *module = obtain_module(desc->plugin_name());
    auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
    auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
    return {creator(desc), deleter};
}
}