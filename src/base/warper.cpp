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
    _marginal->prepare(rp);
    for (auto &v : _conditional_v) {
        v->prepare(rp);
    }
}

//Float Warper2D::func_at(Uint2 coord) const noexcept {
//    Warper *row = _conditional_v[coord.y];
//    return row->func_at(coord.x);
//}
//
//Float Warper2D::PDF(Float2 p) const noexcept {
//    Uint iu = clamp(cast<uint>(p[0] * _conditional_v[0]->size()), 0u, _conditional_v[0]->size() - 1u);
//    Uint iv = clamp(cast<uint>(p[1] * _marginal->size()), 0u, _marginal->size() - 1u);
//    return _conditional_v[iv]->func_at(iu) /
//}

float Warper2D::integral() const noexcept {
    return _marginal->integral();
}

}// namespace vision