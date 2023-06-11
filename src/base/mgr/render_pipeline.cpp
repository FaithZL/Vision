//
// Created by Zero on 04/09/2022.
//

#include "render_pipeline.h"
#include "base/sensor.h"
#include "scene.h"
#include "base/color/spectrum.h"

namespace vision {

RenderPipeline::RenderPipeline(Device *device, ocarina::Context *context)
    : _device(device),
      _context(context),
      _scene(context, this),
      _geometry(this),
      _stream(device->create_stream()),
      _resource_array(device->create_resource_array()) {
    Printer::instance().init(*device);
}

void RenderPipeline::init_postprocessor(const SceneDesc &scene_desc) {
    _postprocessor.set_denoiser(_scene.load<Denoiser>(scene_desc.denoiser_desc));
    _postprocessor.set_tone_mapper(_scene.camera()->film()->tone_mapper());
}

void RenderPipeline::change_resolution(uint2 res) noexcept {
    auto film = _scene.camera()->film();
    film->set_resolution(res);
    film->prepare();
}

void RenderPipeline::prepare_geometry() noexcept {
    for (const Shape *shape : _scene._shapes) {
        shape->fill_geometry(_geometry);
    }
    _geometry.reset_device_buffer();
    _geometry.build_meshes();
    _geometry.upload();
    _geometry.build_accel();
}

void RenderPipeline::prepare_resource_array() noexcept {
    _resource_array.prepare_slotSOA(device());
    _stream << _resource_array->upload_buffer_handles()
            << _resource_array->upload_texture_handles()
            << synchronize() << commit();
}

Spectrum &RenderPipeline::spectrum() noexcept {
    return *_scene.spectrum();
}

const Spectrum &RenderPipeline::spectrum() const noexcept {
    return *_scene.spectrum();
}

void RenderPipeline::deregister_buffer(handle_ty index) noexcept {
    _resource_array->remove_buffer(index);
}

void RenderPipeline::deregister_texture(handle_ty index) noexcept {
    _resource_array->remove_texture(index);
}

void RenderPipeline::compile_shaders() noexcept {
    _scene.integrator()->compile_shader();
}

void RenderPipeline::prepare() {
    _scene.prepare();
    _image_pool.prepare();
    prepare_geometry();
    prepare_resource_array();
    compile_shaders();
}

void RenderPipeline::render(double dt) noexcept {
    Clock clk;
    _scene.integrator()->render();
    double ms = clk.elapse_ms();
    _total_time += ms;
    ++_frame_index;
    cerr << ms << "  " << _total_time / _frame_index << "  " << _frame_index << endl;
    Printer::instance().retrieve_immediately();
}

float4 *RenderPipeline::final_picture() noexcept {
    RegistrableManaged<float4> &frame = _scene.film()->tone_mapped_buffer();
    frame.download_immediately();
    return frame.data();
}

OCHit RenderPipeline::trace_closest(const OCRay &ray) const noexcept {
    return geometry().accel.trace_closest(ray);
}

Bool RenderPipeline::trace_any(const OCRay &ray) const noexcept {
    return geometry().accel.trace_any(ray);
}

Interaction RenderPipeline::compute_surface_interaction(const OCHit &hit, OCRay &ray) const noexcept {
    return geometry().compute_surface_interaction(hit, ray);
}

LightEvalContext RenderPipeline::compute_light_eval_context(const Uint &inst_id, const Uint &prim_id, const Float2 &bary) const noexcept {
    return geometry().compute_light_eval_context(inst_id, prim_id, bary);
}
}// namespace vision