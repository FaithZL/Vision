//
// Created by Zero on 04/09/2022.
//

#include "render_pipeline.h"
#include "base/sensor.h"
#include "base/scene.h"

namespace vision {

RenderPipeline::RenderPipeline(Device *device, vision::Context *context)
    : _device(device),
      _context(context),
      _scene(context) {}

void RenderPipeline::download_result() {
    _scene.film()->copy_to(_render_buffer.get());
}

void RenderPipeline::prepare() noexcept {
    _scene.prepare(this);
    _render_buffer.reset(new_array<float4>(_scene.film()->pixel_num()));
}

void RenderPipeline::build_accel() {
}

void RenderPipeline::render(double dt) {

}

}// namespace vision