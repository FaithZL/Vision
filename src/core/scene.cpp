//
// Created by Zero on 06/09/2022.
//

#include "scene.h"
#include "context.h"

namespace vision {

Scene::Scene(vision::Context *ctx)
    : _context(ctx) {}

void Scene::prepare(const SceneDesc& scene_desc) {
    scene_desc.sensor_desc.ctx = _context;
    auto camera = _context->load_camera(&scene_desc.sensor_desc);
//    static_assert(is_float_element_v<expr_value_t<Var<float>>>);
//    ocarina::cos(Var(0.3f));
//    auto dd =  camera->up(Var(0.f),Var(0.f));
    int i = 0;
}

}// namespace vision