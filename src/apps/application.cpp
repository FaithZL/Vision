//
// Created by Zero on 26/10/2022.
//

#include "application.h"
#include "core/basic_types.h"
#include "ext/imgui/imgui.h"

namespace vision {
using namespace ocarina;
void App::prepare() noexcept {
    context.cli_parser().try_print_help_and_exit();
    scene_desc = context.parse_file();
    rp.init_scene(scene_desc);
    window = context.create_window("vision", rp.resolution(), "gl");
    rp.prepare();
    radiance_image = ImageIO::pure_color(make_float4(0, 0, 0, 1), ColorSpace::LINEAR, rp.resolution());
    register_event();
}

void App::on_key_event(int key, int action) noexcept {
    double dt = window->dt();
    Camera *camera = rp.scene().camera();
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
    need_update = true;
}

void App::on_window_size_change(uint2 size) noexcept {
    rp.change_resolution(size);
}

void App::on_scroll_event(float2 scroll) noexcept {
    need_update = true;
    Camera *camera = rp.scene().camera();
    camera->update_fov_y(scroll.y);
}

void App::update_camera_view(float d_yaw, float d_pitch) noexcept {
    Camera *camera = rp.scene().camera();
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
        double dt = window->dt();
        Camera *camera = rp.scene().camera();
        float3 forward = camera->forward();
        float3 right = camera->right();
        delta *= 0.05f;
        delta.y *= -1;
        float3 dir = forward * delta.y + right * delta.x;
        camera->move(dir);
    } else {
        return;
    }
    need_update = true;
}

void App::on_mouse_event(int button, int action, float2 pos) noexcept {
    switch (button) {
        case 0: left_key_press = bool(action); break;
        case 1: right_key_press = bool(action); break;
    }
}

void App::update(double dt) noexcept {
    rp.upload_data();
    if (need_update) {
        need_update = false;
        rp.update();
    }
    rp.render_to_image(dt, radiance_image.pixel_ptr());
    window->set_background(radiance_image.pixel_ptr<float4>());
    check_and_save();
}

void App::check_and_save() noexcept {
    OutputDesc desc = scene_desc.output_desc;
    if (rp.frame_index() == desc.spp) {
        radiance_image.save(desc.fn);
        if (desc.save_exit) {
            exit(0);
        }
    }
}

int App::run() noexcept {
    window->run([&](double dt) {
        update(dt);
    });
    return 0;
}

void App::register_event() noexcept {
    window->set_key_callback([&]<typename... Args>(Args && ...args) {
        on_key_event(OC_FORWARD(args)...);
    });
    window->set_mouse_callback([&]<typename... Args>(Args && ...args) {
        on_mouse_event(OC_FORWARD(args)...);
    });
    window->set_cursor_position_callback([&]<typename... Args>(Args && ...args) {
        on_cursor_move(OC_FORWARD(args)...);
    });
    window->set_scroll_callback([&]<typename... Args>(Args && ...args) {
        on_scroll_event(OC_FORWARD(args)...);
    });
    //todo check resize bug
//    window->set_window_size_callback([&]<typename... Args>(Args && ...args) {
//        on_window_size_change(OC_FORWARD(args)...);
//    });
}

}// namespace vision