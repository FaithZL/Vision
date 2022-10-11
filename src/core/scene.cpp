//
// Created by Zero on 06/09/2022.
//

#include "scene.h"
#include "context.h"

namespace vision {

Scene::Scene(vision::Context *ctx)
    : _context(ctx) {}

void Scene::prepare(const SceneDesc& scene_desc) {
//    auto f = _context->load_filter(&scene_desc.sensor_desc.filter_desc);

    int i = 0;
}

}// namespace vision