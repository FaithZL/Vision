//
// Created by Zero on 04/09/2022.
//

#include "render_pipeline.h"
#include "base/sensor.h"
#include "base/scene.h"
#include "context.h"

namespace vision {

RenderPipeline::RenderPipeline(Device *device, vision::Context *context)
    : _device(device),
      _context(context),
      _scene(context, this),
      _device_data(device),
      _stream(device->create_stream()) {}

void RenderPipeline::download_result() {
    _scene.film()->copy_to(_render_image.pixel_ptr());
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
    _render_image = ImageIO::pure_color(make_float4(0,0,0,1), ColorSpace::LINEAR, resolution());
}

void RenderPipeline::render(double dt) noexcept {
    Clock clk;
    _scene.integrator()->render(this);
    double ms = clk.elapse_ms();
    _total_time += ms;
    if (_frame_index == 128) {
//        _render_image.save(_context->scene_directory() / "test_cbox.png");
    }
    cout << ms << "  " << _total_time / _frame_index << "  " << _frame_index << endl;
    ++_frame_index;
}
}// namespace vision