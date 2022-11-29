//
// Created by Zero on 29/11/2022.
//

#include "warper.h"
#include "base/render_pipeline.h"
#include "base/scene.h"

namespace vision {

void Warper2D::build(RenderPipeline *rp, vector<float> weights, uint2 res) noexcept {
    _conditional_v.reserve(res.y);
    Scene &scene = rp->scene();
    for (int v = 0; v < res.y; ++v) {
        vector<float> func_v;
        func_v.insert(func_v.end(), &weights[v * res.x], &weights[v * res.x + res.x]);
    }
}

}// namespace vision