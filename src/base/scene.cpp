//
// Created by Zero on 06/09/2022.
//

#include "scene.h"
#include "core/context.h"

namespace vision {

Scene::Scene(vision::Context *ctx)
    : _context(ctx) {}

Node* Scene::load_node(const NodeDesc &desc) {
    const DynamicModule *module = _context->obtain_module(desc.plugin_name());
    auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
    auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
    _all_nodes.emplace_back(creator(desc), deleter);
    return _all_nodes.back().get();
}

void Scene::init(const SceneDesc& scene_desc) {
    scene_desc.light_sampler_desc.scene = this;
    _light_sampler = load<LightSampler>(scene_desc.light_sampler_desc);
    scene_desc.sensor_desc.scene = this;
    _camera = load<Camera>(scene_desc.sensor_desc);
    load_materials(scene_desc.material_descs);
    load_shapes(scene_desc.shape_descs);
    _sampler = load<Sampler>(scene_desc.sampler_desc);
}

void Scene::prepare(RenderPipeline *rp) noexcept {
    _camera->prepare(rp);
    _sampler->prepare(rp);
}

void Scene::load_materials(const vector<MaterialDesc> &material_descs) noexcept {
    for (const MaterialDesc &desc : material_descs) {
        desc.scene = this;
        _materials.push_back(load<Material>(desc));
    }
}

void Scene::load_shapes(const vector<ShapeDesc> &descs) noexcept {
    for (const ShapeDesc &desc : descs) {
        desc.scene = this;
        _shapes.push_back(load<Shape>(desc));
    }
}

Light *Scene::load_light(const LightDesc &desc) noexcept {
    OC_ASSERT(_light_sampler != nullptr);
    auto ret = load<Light>(desc);
    _light_sampler->add_light(ret);
    return ret;
}

void Scene::load_lights(const vector<LightDesc> &descs) noexcept {
    for (const LightDesc &desc : descs) {
        desc.scene = this;
        load_light(desc);
    }
}

}// namespace vision