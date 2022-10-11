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

void Context::prepare() noexcept {
    auto scene_desc = SceneDesc::from_json(cli_parser().scene_file());

    _scene.prepare(std::move(scene_desc));
}

Node* Context::load_node(NodeDesc *desc) {
    const DynamicModule *module = obtain_module(desc->plugin_name());
    auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
    auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
    _all_nodes.emplace_back(creator(desc), deleter);
    return _all_nodes.back().get();
}

Filter *Context::load_filter(FilterDesc *desc) {
    return load<Filter>(desc);
}

}// namespace vision