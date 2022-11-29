//
// Created by Zero on 29/11/2022.
//

#include "warper.h"
#include "base/render_pipeline.h"
#include "base/scene.h"

namespace vision {

void Warper2D::build(RenderPipeline *rp, vector<float> weights, uint2 res) noexcept {
    TIMER_TAG(build_env2d, "build env map 2d");
    _conditional_v.reserve(res.y);
    Scene &scene = rp->scene();
    auto iter = weights.cbegin();
    for (int v = 0; v < res.y; ++v) {
        vector<float> func_v;
        func_v.insert(func_v.end(), iter, iter + res.x);
        iter += res.x;
        Warper *warper = scene.load_warper();
        warper->build(func_v);
        _conditional_v.push_back(warper);
    }
    vector<float> marginal_func;
    marginal_func.reserve(res.y);
    for (int v = 0; v < res.y; ++v) {
        marginal_func.push_back(_conditional_v[v]->integral());
    }
    _marginal = scene.load_warper();
    _marginal->build(marginal_func);
}

void Warper2D::prepare(RenderPipeline *rp) noexcept {
    int i = 0;
}

}// namespace vision