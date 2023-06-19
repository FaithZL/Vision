//
// Created by Zero on 26/10/2022.
//

#include "application.h"
#include "core/basic_types.h"
#include "ext/imgui/imgui.h"
#include "base/mgr/global.h"

namespace vision {
using namespace ocarina;

void App::init(int argc) {
    core::log_level_info();
    if (argc == 1 || cli_parser->has_help_cmd()) {
        cli_parser->print_help();
        exit(0);
    }
    Global::instance().set_pipeline(rp);
    params.init(cli_parser.get());
    device.init_rtx();
    if (params.clear_cache) {
        Context::instance().clear_cache();
    }
    if (cli_parser) {
        cli_parser->try_print_help_and_exit();
    }
    prepare();
}

void App::init_pipeline(const SceneDesc &desc) {
    desc.pipeline_desc.device = &device;
    rp = Global::node_mgr().load<Pipeline>(desc.pipeline_desc);
    Global::instance().set_pipeline(rp);
    pipeline().init_scene(desc);
}

void App::prepare() {
    scene_desc = SceneDesc::from_json(params.scene_file);
    init_pipeline(scene_desc);
    pipeline().prepare();
    window = Context::instance().create_window("LajiRender", pipeline().resolution(), "gl");
    register_event();
}

void App::on_key_event(int key, int action) noexcept {
    double dt = window->dt();
    Camera *camera = pipeline().scene().camera();
    float3 forward = camera->forward();
    float3 up = camera->up();
    float3 right = camera->right();
    float distance = camera->velocity() * dt;
    switch (key) {
        case 'W':
            camera->move(forward * distance);
            break;
        case 'A':
            camera->move(-right * distance);
            break;
        case 'S':
            camera->move(-forward * distance);
            break;
        case 'D':
            camera->move(right * distance);
            break;
        case 'Q':
            camera->move(-up * distance);
            break;
        case 'E':
            camera->move(up * distance);
            break;
        default:
            return;
    }
    invalidation = true;
}

void App::on_window_size_change(uint2 size) noexcept {
    pipeline().change_resolution(size);
}

void App::on_scroll_event(float2 scroll) noexcept {
    invalidation = true;
    Camera *camera = pipeline().scene().camera();
    camera->update_fov_y(scroll.y);
}

void App::update_camera_view(float d_yaw, float d_pitch) noexcept {
    Camera *camera = pipeline().scene().camera();
    float sensitivity = camera->sensitivity();
    camera->update_yaw(d_yaw * sensitivity);
    camera->update_pitch(d_pitch * sensitivity);
}

void App::on_cursor_move(float2 pos) noexcept {
    if (is_zero(last_cursor_pos)) {
        last_cursor_pos = pos;
        return;
    }
    float2 delta = pos - last_cursor_pos;
    last_cursor_pos = pos;
    if (right_key_press) {
        update_camera_view(delta.x, -delta.y);
    } else if (left_key_press) {
        Camera *camera = pipeline().scene().camera();
        float3 forward = camera->forward();
        float3 right = camera->right();
        delta *= 0.05f;
        delta.y *= -1;
        float3 dir = forward * delta.y + right * delta.x;
        camera->move(dir);
    } else {
        return;
    }
    invalidation = true;
}

void App::on_mouse_event(int button, int action, float2 pos) noexcept {
    switch (button) {
        case 0: left_key_press = bool(action); break;
        case 1: right_key_press = bool(action); break;
        case 2: need_save = bool(action); break;
        default: break;
    }
}

void App::update(double dt) noexcept {
    pipeline().upload_data();
    if (invalidation) {
        invalidation = false;
        pipeline().invalidate();
    }
    pipeline().render(dt);
    auto &view_buffer = pipeline().view_buffer();
    view_buffer.download_immediately();
    window->set_background(view_buffer.data());
    check_and_save();
}

void App::check_and_save() noexcept {
    OutputDesc desc = scene_desc.output_desc;
    if (pipeline().frame_index() == desc.spp || need_save) {
        save_result();
    }
}

void App::save_result() noexcept {
    OutputDesc desc = scene_desc.output_desc;
    ImageIO::save_image(Global::instance().scene_path() / desc.fn, PixelStorage::FLOAT4,
                        pipeline().resolution(), pipeline().final_picture());
    if (desc.save_exit) {
        exit(0);
    }
    need_save = false;
}

int App::run() noexcept {
    window->run([&](double dt) {
        update(dt);
    });
    return 0;
}

void App::register_event() noexcept {
    window->set_key_callback([&]<typename... Args>(Args &&...args) {
        on_key_event(OC_FORWARD(args)...);
    });
    window->set_mouse_callback([&]<typename... Args>(Args &&...args) {
        on_mouse_event(OC_FORWARD(args)...);
    });
    window->set_cursor_position_callback([&]<typename... Args>(Args &&...args) {
        on_cursor_move(OC_FORWARD(args)...);
    });
    window->set_scroll_callback([&]<typename... Args>(Args &&...args) {
        on_scroll_event(OC_FORWARD(args)...);
    });
    //todo check resize bug
    //    window->set_window_size_callback([&]<typename... Args>(Args && ...args) {
    //        on_window_size_change(OC_FORWARD(args)...);
    //    });
}

}// namespace vision