//
// Created by Zero on 26/10/2022.
//

#include "application.h"
#include "GUI/window.h"
#include "math/basic_types.h"
#include "base/mgr/global.h"
#include "base/importer.h"
#include "rhi/stats.h"

#define VS_KEY_RIGHT 262
#define VS_KEY_LEFT 263
#define VS_KEY_DOWN 264
#define VS_KEY_UP 265

namespace vision {
using namespace ocarina;

void App::init(int argc) {
    core::log_level_info();
    if (argc == 1 || cli_parser->has_help_cmd()) {
        cli_parser->print_help();
        exit(0);
    }
    params.init(cli_parser.get());
    device.init_rtx();
    if (params.clear_cache) {
        FileManager::instance().clear_cache();
    }
    if (cli_parser) {
        cli_parser->try_print_help_and_exit();
    }
    prepare();
}

void App::init_pipeline() {
    Global::instance().set_device(&device);
    rp = Importer::import_scene(params.scene_file);
    pipeline().init();
    view_buffer.resize(pipeline().pixel_num());
}

void App::prepare() {
    init_pipeline();
    pipeline().prepare();
    window = FileManager::instance().create_window("LajiRender", pipeline().resolution(), "imGui", true);
    register_event();
}

void App::on_key_event(int key, int action) noexcept {
    switch (key) {
        case 'F':
            key_f_press = bool(action);
            return;
        case 'R':
            key_r_press = bool(action);
            return;
        case 'G':
            key_g_press = bool(action);
            if (key_g_press) {
                Env::debugger().filp_enabled();
                cout << ocarina::format("\n Debugger state is {}", Env::debugger().is_enabled()) << endl;
            }
            return;
        case 'Z':
            if (action) {
                pipeline().filp_show_fps();
            }
            return;
        default:
            break;
    }
    if (action == 0) {
        return;
    }
    double dt = window->dt();
    TSensor camera = pipeline().scene().sensor();
    float3 forward = camera->forward();
    float3 up = camera->up();
    float3 right = camera->right();
    float distance = camera->velocity() * dt;
    float sens = 1.f;
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
        case VS_KEY_UP:
            update_camera_view(0, sens);
            break;
        case VS_KEY_DOWN:
            update_camera_view(0, -sens);
            break;
        case VS_KEY_LEFT:
            update_camera_view(-sens, 0);
            break;
        case VS_KEY_RIGHT:
            update_camera_view(sens, 0);
            break;
        default:
            return;
    }
    invalidation = true;
}

void App::on_window_size_change(uint2 size) noexcept {
    pipeline().change_resolution(size);
    invalidation = true;
}

void App::on_scroll_event(float2 scroll) noexcept {
    invalidation = true;
    auto camera = pipeline().scene().sensor();
    camera->update_fov_y(scroll.y);
    invalidation = true;
}

void App::update_camera_view(float d_yaw, float d_pitch) const noexcept {
    auto camera = pipeline().scene().sensor();
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
    } else {
        return;
    }
    invalidation = true;
}

void App::on_mouse_event(int button, int action, float2 pos) noexcept {
    switch (button) {
        case 0: {
            left_key_press = bool(action);
            switch (action) {
                case 1:
                    pipeline().on_touch(make_uint2(pos));
                    break;
                    //                case 0: Env::debugger().set_upper(make_uint2(pos)); break;
                default: break;
            }
            break;
        }
        case 1: right_key_press = bool(action); break;
        default: break;
    }
}

bool App::render_UI(ocarina::Widgets *widgets) noexcept {
    pipeline().render_UI(widgets);
    return true;
}

void App::update(double dt) noexcept {
    pipeline().upload_data();
    HotfixSystem::instance().execute_callback();
    if (invalidation || pipeline().has_changed()) {
        auto camera = pipeline().scene().sensor();
        //        OC_INFO(camera->to_string());
        invalidation = false;
        pipeline().invalidate();
    }
    pipeline().display(dt);
    pipeline().reset_status();
    render_UI(window->widgets());
    window->set_background(pipeline().frame_buffer()->window_buffer().data());
    rp->check_and_save();
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
    window->set_window_size_callback([&]<typename... Args>(Args &&...args) {
        on_window_size_change(OC_FORWARD(args)...);
    });
}

}// namespace vision