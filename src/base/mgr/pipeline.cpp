//
// Created by Zero on 04/09/2022.
//

#include "pipeline.h"
#include "base/sensor/sensor.h"
#include "scene.h"
#include "base/color/spectrum.h"
#include "GUI/window.h"

namespace vision {

Pipeline::Pipeline(const vision::PipelineDesc &desc)
    : Node(desc),
      _device(&Global::instance().device()),
      _geometry(this),
      _stream(device().create_stream()),
      _bindless_array(device().create_bindless_array()) {
    Global::instance().set_pipeline(this);
    Env::printer().init(device());
    Env::debugger().init(device());
    Env::set_code_obfuscation(desc["obfuscation"].as_bool(false));
    Env::set_valid_check(desc["valid_check"].as_bool(false));
    _frame_buffer = NodeMgr::instance().load<FrameBuffer>(desc.frame_buffer_desc);
}

bool Pipeline::has_changed() noexcept {
    return _changed || _scene.has_changed();
}

bool Pipeline::render_UI(ocarina::Widgets *widgets) noexcept {
    widgets->use_window("pipeline stats", [&]() {
        widgets->text("pipeline type: %s", impl_type().data());
        widgets->text("current frame: %.3f\naverage: %.3f\nframe index: %u",
                      cur_render_time(),
                      render_time() / frame_index(),
                      frame_index());
    });
    return _scene.render_UI(widgets);
}

const Buffer<float4> &Pipeline::view_buffer() {
    return scene().film()->output_buffer().device_buffer();
}

void Pipeline::change_resolution(uint2 res) noexcept {
    auto film = _scene.camera()->film();
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
    _scene.clear_shapes();
    MeshRegistry::instance().clear();
}

void Pipeline::upload_bindless_array() noexcept {
    _stream << _bindless_array.update_slotSOA() << synchronize() << commit();
    _stream << _bindless_array.upload_handles() << synchronize() << commit();
}

void Pipeline::deregister_buffer(handle_ty index) noexcept {
    _bindless_array->remove_buffer(index);
}

void Pipeline::deregister_texture(handle_ty index) noexcept {
    _bindless_array->remove_texture(index);
}

void Pipeline::before_render() noexcept {
}

void Pipeline::after_render() noexcept {
    scene().camera()->after_render();
}

void Pipeline::display(double dt) noexcept {
    Clock clk;
    before_render();
    render(dt);
    after_render();
    double ms = clk.elapse_ms();
    integrator()->accumulate_render_time(ms);
    Env::printer().retrieve_immediately();
}

float4 *Pipeline::final_picture(const OutputDesc &desc) noexcept {
    RegistrableManaged<float4> &original = _scene.film()->rt_buffer();
    bool gamma = !(desc.fn.ends_with("exr") || desc.fn.ends_with("hdr"));
    if (desc.denoise) {
        OfflineDenoiseInput input;
        input.resolution = resolution();
        input.output = &_final_picture;
        input.color = &original;
        _postprocessor.denoise(input);
        _postprocessor.tone_mapping(_final_picture, _final_picture, gamma);
    } else {
        _postprocessor.tone_mapping(original, _final_picture, gamma);
    }
    _final_picture.download_immediately();
    return _final_picture.data();
}

}// namespace vision