//
// Created by Zero on 26/10/2022.
//

#include "application.h"
#include "core/basic_types.h"

namespace vision {
using namespace ocarina;
void App::prepare() noexcept {
    scene_desc = context.parse_file();
    rp.init_scene(scene_desc);
    window = context.create_window("vision", rp.resolution(), "gl");
    rp.prepare();
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
            break;
    }
}

void App::on_scroll_event(float2 scroll) noexcept {
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
    cout << pos.x << endl;
    int2 delta = make_int2(pos) - _last_cursor_pos;
    if (_right_key_press && nonzero(_last_cursor_pos)) {
        update_camera_view(delta.x, -delta.y);
    }
    _last_cursor_pos = make_int2(pos);
}


void App::on_mouse_event(int button, int action, float2 pos) noexcept {
    if (button == 0) {
        _left_key_press = bool(action);
    }
    if (button == 1) {
        _right_key_press = bool(action);
    }
}

void App::update(double dt) noexcept {
    rp.update();
    rp.render(dt);
    rp.download_result();
    window->set_background(rp.buffer());
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
}

}// namespace vision