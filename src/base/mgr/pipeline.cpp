//
// Created by Zero on 04/09/2022.
//

#include "pipeline.h"
#include "base/sensor/photosensory.h"
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
    frame_buffer_ = Node::create_shared<FrameBuffer>(desc.frame_buffer_desc);
}

void Pipeline::init() noexcept {
    frame_buffer_->resize(resolution());
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
    stream_ << frame_buffer_->compute_hit(0);
    stream_ << frame_buffer()->hit_buffer().download(index, 1);
    stream_ << synchronize() << commit();
    TriangleHit hit = buffer[index];

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
        widgets->check_box("stats", &show_stats_);
        widgets->check_box("hotfix", &show_hotfix_);
        widgets->check_box("output setting", &show_output_);
        widgets->button_click("clear cache", [&] {
            FileManager::instance().clear_cache();
        });
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
    if (show_stats_) {
        render_stats(widgets);
    }
    if (show_hotfix_) {
        render_hotfix(widgets);
    }
    if (show_output_) {
        render_output(widgets);
    }
    return true;
}

struct FileNameOption {
    static constexpr auto len = 256;
    char data[len] = {};
    int ext_index{0};
    vector<const char *> exts = {".exr", ".png", ".jpg", ".hdr"};
    explicit FileNameOption(const fs::path &fn) {
        auto stem = fn.stem().string();
        for (int i = 0; i < stem.size(); ++i) {
            data[i] = stem[i];
        }
    }
    void render_UI(Widgets *widgets,
                   OutputDesc &output_desc) noexcept {
        widgets->input_text("fn", data, len);
        widgets->combo("ext", &ext_index, exts);
        string ext = exts[ext_index];
        string stem = data;
        output_desc.fn = stem + ext;
    }
};

void Pipeline::render_output(ocarina::Widgets *widgets) noexcept {
    widgets->use_window("output setting", [&] {
        widgets->drag_uint("frame num", &output_desc.spp, 10, 1, InvalidUI32);
        widgets->check_box("exit after save", &output_desc.save_exit);
        widgets->check_box("use denoise", &output_desc.denoise);
        fs::path scene_path = Global::instance().scene_path();
        static FileNameOption fnp{output_desc.fn};
        fnp.render_UI(widgets, output_desc);
        widgets->button_click("save", [&] {
            need_save_ = true;
        });
    });
}

void Pipeline::render_detail(ocarina::Widgets *widgets) noexcept {
    if (cur_node_ == nullptr) {
        return;
    }
    widgets->use_window("detail", [&] {
        cur_node_->render_UI(widgets);
    });
}

namespace {
void hotfix_window(ocarina::Widgets *widgets) noexcept {
    HotfixSystem &hotfix = HotfixSystem::instance();
    if (hotfix.is_working()) {
        return;
    }
    widgets->button_click("reload", [&] {
        bool update = hotfix.check_and_build();
        if (!update) {
            OC_INFO("no modification");
        }
    });

    widgets->set_enabled(hotfix.has_previous(), [&] {
        widgets->button_click("previous", [&] {
            hotfix.previous_version();
        });
    });

    widgets->set_enabled(hotfix.has_next(), [&] {
        widgets->button_click("next", [&] {
            hotfix.next_version();
        });
    });
}
}// namespace

void Pipeline::render_hotfix(ocarina::Widgets *widgets) noexcept {
    widgets->use_window("hotfix system", [&] {
        hotfix_window(widgets);
    });
}

void Pipeline::save_result() noexcept {
    OutputDesc desc = output_desc;
    Image::save_image(Global::instance().scene_path() / desc.fn, PixelStorage::FLOAT4,
                      resolution(), final_picture(desc));
    if (desc.save_exit) {
        exit(0);
    }
    need_save_ = false;
}

void Pipeline::check_and_save() noexcept {
    if ((frame_index() == output_desc.spp && output_desc.spp != 0) || need_save_) {
        save_result();
    }
}

void Pipeline::update_runtime_object(const vision::IObjectConstructor *constructor) noexcept {
    std::tuple tp = {addressof(frame_buffer_)};
    HotfixSystem::replace_objects(constructor, tp);
}

void Pipeline::render_stats(ocarina::Widgets *widgets) noexcept {
    auto tex_size = MemoryStats::instance().tex_size();
    auto buffer_size = MemoryStats::instance().buffer_size();
    size_t tex_slot_mem_size = bindless_array()->tex_slots_size();
    size_t buffer_slot_mem_size = bindless_array()->buffer_slots_size();
    auto label = ocarina::format("memory stats total is {}", bytes_string(tex_size + buffer_size +
                                                                          tex_slot_mem_size + buffer_slot_mem_size));
    widgets->use_window(label, [&] {
        widgets->use_folding_header("texture stats", [&] {
            widgets->text("total texture size is %s", bytes_string(tex_size).c_str());
            MemoryStats::instance().foreach_tex_info([&](auto data) {
                double percent = double(data.size()) / tex_size;
                widgets->text(ocarina::format("size {}, percent {:.2f} %%, tex name {}\n",
                                              bytes_string(data.size()), percent * 100, data.name));
            });
        });

        widgets->use_folding_header("buffer stats", [&] {
            widgets->text("total buffer size is %s", bytes_string(buffer_size).c_str());
            MemoryStats::instance().foreach_buffer_info([&](auto data) {
                double percent = double(data.size) / buffer_size;
                widgets->text(ocarina::format("size {}, percent {:.2f} %%, block {}\n",
                                              bytes_string(data.size), percent * 100,
                                              data.name));
            });
        });

        widgets->use_folding_header("mesh stats", [&] {
            auto triangle_num = geometry_.accel().triangle_num();
            auto vert_num = geometry_.accel().vertex_num();
            auto mesh_num = geometry_.accel().mesh_num();
            auto string = ocarina::format("vertex num is {}\ntriangle num is {}\nmesh num is {}",
                                          triangle_num, vert_num, mesh_num);
            widgets->text(string);
        });

        widgets->use_folding_header("bindless_array stats", [&] {
            uint bindless_buffer_num = bindless_array().buffer_num();
            uint bindless_tex_num = bindless_array().texture_num();
            size_t max_slot_num = BindlessArray::max_slot_num();

            widgets->text(ocarina::format("bindless buffer num is {}", bindless_buffer_num));
            widgets->text(ocarina::format("bindless tex num is {}", bindless_tex_num));
            widgets->text(ocarina::format("max slot num is {}", max_slot_num));
            widgets->text(ocarina::format("bindless slot men size is {}", bytes_string(tex_slot_mem_size + buffer_slot_mem_size)));
        });
    });
}

const Buffer<float4> &Pipeline::view_buffer() {
    return frame_buffer_->view_buffer();
}

void Pipeline::change_resolution(uint2 res) noexcept {
    if (all(res == resolution())) { return; }
    scene_.update_resolution(res);
    frame_buffer_->update_resolution(res);
    final_picture_.reset_all(device(), pixel_num(), "offline final picture");
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
    stream_ << Env::debugger().upload();
}

void Pipeline::after_render() noexcept {
    Env::debugger().reset_range();
    scene().sensor()->after_render();
    frame_buffer_->after_render();
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
    RegistrableManaged<float4> &original = scene_.rad_collector()->output_buffer();
    bool gamma = !(desc.fn.ends_with("exr") || desc.fn.ends_with("hdr"));
    if (desc.denoise) {
        OfflineDenoiseInput input;
        input.resolution = resolution();
        input.output = &final_picture_;
        input.color = &original;
        input.normal = &frame_buffer_->normal();
        input.albedo = &frame_buffer_->albedo();
        postprocessor_.denoise(input);
        postprocessor_.tone_mapping(final_picture_, final_picture_, gamma);
    } else {
        postprocessor_.tone_mapping(original, final_picture_, gamma);
    }
    final_picture_.download_immediately();
    return final_picture_.data();
}

}// namespace vision