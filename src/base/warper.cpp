//
// Created by Zero on 29/11/2022.
//

#include "warper.h"
#include "base/render_pipeline.h"
#include "base/scene.h"

namespace vision {

void Warper2D::build(RenderPipeline *rp, vector<float> weights, uint2 res) noexcept {
    TIMER_TAG(build_env2d, "build env map 2d");
    Scene &scene = rp->scene();
    _impl = scene.load_warper();
    _impl->build(move(weights));
    _resolution = res;
}

void Warper2D::prepare(RenderPipeline *rp) noexcept {
    _impl->prepare(rp);
}

Float Warper2D::func_at(Uint2 coord) const noexcept {
    Uint idx = _resolution.x * coord.y + coord.x;
    return _impl->func_at(idx);
}

Float Warper2D::PDF(Float2 p) const noexcept {
    Uint idx = _resolution.x * cast<uint>(p.x) + cast<uint>(p.y);
    return _impl->PDF(idx);
}



float Warper2D::integral() const noexcept {
    return _impl->integral();
}

}// namespace vision