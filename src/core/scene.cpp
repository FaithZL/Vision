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
    _camera = load<Camera>(&scene_desc.sensor_desc);
    load_shapes(scene_desc.shape_descs);
    _light_sampler = load<LightSampler>(&scene_desc.light_sampler_desc);
    _sampler = load<Sampler>(&scene_desc.sampler_desc);
    load_materials(scene_desc.material_descs);
}

void Scene::prepare(RenderPipeline *rp) noexcept {
    _camera->prepare(rp);
    _sampler->prepare(rp);
}

}// namespace vision