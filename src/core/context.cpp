//
// Created by Zero on 05/10/2022.
//

#include "context.h"

namespace vision {

Context::Context(int argc, char **argv, ocarina::string_view cache_dir)
    : Super(fs::path(argv[0]).parent_path(), cache_dir),
      _cli_parser(argc, argv) {
}

Context::PluginHandle Context::load_plugin(Description *desc) {
    const DynamicModule *module = obtain_module(desc->plugin_name());
    auto creator = reinterpret_cast<PluginCreator *>(module->function_ptr("create"));
    auto deleter = reinterpret_cast<PluginDeleter *>(module->function_ptr("destroy"));
    return PluginHandle(creator(desc), deleter);
}
}