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
      device_(&Global::instance().device()),
      geometry_(this),
      stream_(device().create_stream()),
      bindless_array_(device().create_bindless_array()) {
    Global::instance().set_pipeline(this);
    Env::printer().init(device());
    Env::debugger().init(device());
    Env::set_code_obfuscation(desc["obfuscation"].as_bool(false));
    Env::set_valid_check(desc["valid_check"].as_bool(false));
    frame_buffer_ = Node::load<FrameBuffer>(desc.frame_buffer_desc);
}

void Pipeline::prepare() noexcept {
    frame_buffer_->prepare();
}

void Pipeline::reset_status() noexcept {
    GUI::reset_status();
    scene_.reset_status();
}

void Pipeline::on_touch(ocarina::uint2 pos) noexcept {
    Env::debugger().set_lower(pos);

    auto &buffer = frame_buffer_->hit_buffer();
    uint index = frame_buffer()->pixel_index(pos);
    stream_ << frame_buffer_->compute_hit();
    stream_ << frame_buffer()->hit_buffer().download(index, 1);
    stream_ << synchronize() << commit();
    Hit hit = buffer[index];

    scene_.mark_selected(hit);
}

bool Pipeline::has_changed() noexcept {
    return changed_ || scene_.has_changed();
}

bool Pipeline::render_UI(ocarina::Widgets *widgets) noexcept {
    widgets->use_window("render stats", [&]() {
        widgets->text("pipeline type: %s", impl_type().data());
        widgets->text("current frame: %.3f\naverage: %.3f\nframe index: %u",
                      cur_render_time(),
                      render_time() / frame_index(),
                      frame_index());
        widgets->check_box("scene data", &show_scene_data_);
        widgets->check_box("detail", &show_detail_);
        widgets->check_box("framebuffer", &show_framebuffer_data_);
    });
    if (show_scene_data_) {
        scene_.render_UI(widgets);
    }
    if (show_detail_) {
        render_detail(widgets);
    }
    if (show_framebuffer_data_) {
        widgets->use_window("frame buffer", [&] {
            frame_buffer_->render_UI(widgets);
        });
    }
    return true;
}

void Pipeline::render_detail(ocarina::Widgets *widgets) noexcept {
    if (cur_node_ == nullptr) {
        return;
    }
    widgets->use_window("detail", [&] {
        cur_node_->render_UI(widgets);
    });
}

const Buffer<float4> &Pipeline::view_buffer() {
    return frame_buffer_->view_buffer();
}

void Pipeline::change_resolution(uint2 res) noexcept {
    auto film = scene_.camera()->film();
    film->set_resolution(res);
    film->prepare();
}

void Pipeline::prepare_geometry() noexcept {
    geometry_.update_instances(scene_.instances());
    geometry_.reset_device_buffer();
    geometry_.upload();
    geometry_.build_accel();
}

void Pipeline::update_geometry() noexcept {
    geometry_.update_instances(scene_.instances());
    geometry_.upload();
}

void Pipeline::clear_geometry() noexcept {
    geometry_.clear();
    scene_.clear_shapes();
    MeshRegistry::instance().clear();
}

void Pipeline::upload_bindless_array() noexcept {
    stream_ << bindless_array_.update_slotSOA() << synchronize() << commit();
    stream_ << bindless_array_.upload_handles() << synchronize() << commit();
}

void Pipeline::deregister_buffer(handle_ty index) noexcept {
    bindless_array_->remove_buffer(index);
}

void Pipeline::deregister_texture(handle_ty index) noexcept {
    bindless_array_->remove_texture(index);
}

void Pipeline::before_render() noexcept {
}

void Pipeline::after_render() noexcept {
    Env::debugger().reset_range();
    scene().camera()->after_render();
}

void Pipeline::commit_command() noexcept {
    stream_ << frame_buffer_->gamma_correct();
    stream_ << synchronize();
    stream_ << commit();
}

void Pipeline::display(double dt) noexcept {
    Clock clk;
    before_render();
    render(dt);
    commit_command();
    after_render();
    double ms = clk.elapse_ms();
    integrator()->accumulate_render_time(ms);
    Env::printer().retrieve_immediately();
}

float4 *Pipeline::final_picture(const OutputDesc &desc) noexcept {
    RegistrableManaged<float4> &original = scene_.film()->rt_buffer();
    bool gamma = !(desc.fn.ends_with("exr") || desc.fn.ends_with("hdr"));
    if (desc.denoise) {
        OfflineDenoiseInput input;
        input.resolution = resolution();
        input.output = &final_picture_;
        input.color = &original;
        postprocessor_.denoise(input);
        postprocessor_.tone_mapping(final_picture_, final_picture_, gamma);
    } else {
        postprocessor_.tone_mapping(original, final_picture_, gamma);
    }
    final_picture_.download_immediately();
    return final_picture_.data();
}

}// namespace vision