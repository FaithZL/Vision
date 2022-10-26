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
      _scene(context),
      _device_data(device),
      _stream(device->create_stream()) {}

void RenderPipeline::download_result() {
    _scene.film()->copy_to(_render_buffer.get());
}

void RenderPipeline::prepare_device_data() noexcept {
    for (const Shape *shape : _scene._shapes) {
        shape->fill_device_data(_device_data);
    }
    _device_data.reset_device_buffer();
    _device_data.build_meshes();
    _device_data.upload();
    _device_data.build_accel();
}

void RenderPipeline::compile_shaders() noexcept {
    _scene.integrator()->compile_shader(this);
}

void RenderPipeline::prepare() noexcept {
    _scene.prepare(this);
    prepare_device_data();
    compile_shaders();
    _render_buffer.reset(new_array<float4>(_scene.film()->pixel_num()));
}

void RenderPipeline::render(double dt) {
    _scene.integrator()->render(this);
}
}// namespace vision