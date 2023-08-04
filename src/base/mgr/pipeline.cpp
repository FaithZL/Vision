//
// Created by Zero on 04/09/2022.
//

#include "pipeline.h"
#include "base/sensor/sensor.h"
#include "scene.h"
#include "base/color/spectrum.h"

namespace vision {

Pipeline::Pipeline(Device *device)
    : _device(device),
      _geometry(this),
      _stream(device->create_stream()),
      _resource_array(device->create_resource_array()) {
    Printer::instance().init(*device);
}

Pipeline::Pipeline(const vision::PipelineDesc &desc)
    : Node(desc),
      _device(desc.device),
      _geometry(this),
      _stream(device().create_stream()),
      _resource_array(device().create_resource_array()) {
    Printer::instance().init(device());
}

const Buffer<float4> &Pipeline::view_buffer() {
    return scene().radiance_film()->tone_mapped_buffer().device_buffer();
}

void Pipeline::change_resolution(uint2 res) noexcept {
    auto film = _scene.camera()->radiance_film();
    film->set_resolution(res);
    film->prepare();
}

void Pipeline::prepare_geometry() noexcept {
    _geometry.update_instances(_scene.instances());
    _geometry.reset_device_buffer();
    _geometry.upload();
    _geometry.build_accel();
}

void Pipeline::update_geometry() noexcept {
    _geometry.update_instances(_scene.instances());
    _geometry.upload();
}

void Pipeline::clear_geometry() noexcept {
    _geometry.clear();
}

void Pipeline::upload_resource_array() noexcept {
    _stream << _resource_array.update_slotSOA() << synchronize() << commit();
    _stream << _resource_array.upload_handles() << synchronize() << commit();
}

void Pipeline::deregister_buffer(handle_ty index) noexcept {
    _resource_array->remove_buffer(index);
}

void Pipeline::deregister_texture(handle_ty index) noexcept {
    _resource_array->remove_texture(index);
}

float4 *Pipeline::final_picture(bool denoise) noexcept {
    RegistrableManaged<float4> &original = _scene.radiance_film()->original_buffer();
    if (denoise) {
        _postprocessor.denoise(resolution(), &_final_picture, &original, nullptr, nullptr);
        _postprocessor.tone_mapping(_final_picture, _final_picture);
    } else {
        _postprocessor.tone_mapping(original, _final_picture);
    }
    _final_picture.download_immediately();
    return _final_picture.data();
}

OCHit Pipeline::trace_closest(const OCRay &ray) const noexcept {
    return geometry().accel.trace_closest(ray);
}

Bool Pipeline::trace_any(const OCRay &ray) const noexcept {
    return geometry().accel.trace_any(ray);
}

Interaction Pipeline::compute_surface_interaction(const OCHit &hit, OCRay &ray) const noexcept {
    return geometry().compute_surface_interaction(hit, ray);
}

LightEvalContext Pipeline::compute_light_eval_context(const Uint &inst_id, const Uint &prim_id, const Float2 &bary) const noexcept {
    return geometry().compute_light_eval_context(inst_id, prim_id, bary);
}
}// namespace vision