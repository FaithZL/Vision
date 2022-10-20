//
// Created by Zero on 06/09/2022.
//

#include "scene.h"
#include "context.h"

namespace vision {

Scene::Scene(vision::Context *ctx)
    : _context(ctx) {}

Node* Scene::load_node(const NodeDesc *desc) {
    const DynamicModule *module = _context->obtain_module(desc->plugin_name());
    auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
    auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
    _all_nodes.emplace_back(creator(desc), deleter);
    return _all_nodes.back().get();
}

void Scene::init(const SceneDesc& scene_desc) {
    scene_desc.sensor_desc.scene = this;
    _camera = load_camera(&scene_desc.sensor_desc);
}

void Scene::prepare(RenderPipeline *rp) noexcept {
    _camera->prepare(rp);
}

}// namespace vision