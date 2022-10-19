//
// Created by Zero on 06/09/2022.
//

#include "scene.h"
#include "context.h"

namespace vision {

Scene::Scene(vision::Context *ctx)
    : _context(ctx) {}

void Scene::init(const SceneDesc& scene_desc) {
    scene_desc.sensor_desc.ctx = _context;
    _camera = _context->load_camera(&scene_desc.sensor_desc);

}

void Scene::prepare(RenderPipeline *rp) noexcept {
    _camera->prepare(rp);
}

}// namespace vision