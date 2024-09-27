//
// Created by Zero on 2024/9/21.
//

#include "visualizer.h"
#include "base/sensor/camera.h"
#include "mgr/pipeline.h"

namespace vision {

Camera *Visualizer::camera() const noexcept {
    return scene().camera().get();
}

uint2 Visualizer::resolution() const noexcept {
    return pipeline()->resolution();
}

void Visualizer::init() noexcept {
    line_segments_ = device().create_managed_list<LineSegment>(10000, "line segments");
}

void Visualizer::line_bresenham(float2 p1, float2 p2, float4 val, float4 *pixel) const noexcept {
    int px1 = p1.x;
    int py1 = p1.y;

    int px2 = p2.x;
    int py2 = p2.y;

    if (px1 == px2 && py1 == py2) {
        write(px1, py1, val, pixel);
    }

    int dx = ocarina::abs(px2 - px1);
    int dy = ocarina::abs(py2 - py1);

    if (dx >= dy) {
        if (px1 > px2) {
            std::swap(p1, p2);
        }
        px1 = p1.x;
        py1 = p1.y;

        px2 = p2.x;
        py2 = p2.y;

        int sign = py2 >= py1 ? 1 : -1;
        int k = sign * dy * 2;
        int e = -dx * sign;

        for (int x = px1, y = py1; x <= px2; ++x) {
            write(x, y, val, pixel);
            e += k;
            if (sign * e > 0) {
                y += sign;
                e -= 2 * dx * sign;
            }
        }
    } else {
        if (py1 > py2) {
            std::swap(p1, p2);
        }
        px1 = p1.x;
        py1 = p1.y;

        px2 = p2.x;
        py2 = p2.y;

        int sign = px2 > px1 ? 1 : -1;
        int k = sign * dx * 2;
        int e = -dy * sign;

        for (int x = px1, y = py1; y <= py2; ++y) {
            write(x, y, val, pixel);
            e += k;
            if (sign * e > 0) {
                x += sign;
                e -= 2 * dy * sign;
            }
        }
    }
}

void Visualizer::write(int x, int y, ocarina::float4 val, ocarina::float4 *pixel) const noexcept {
    uint2 res = resolution();
    if (x >= res.x || x < 0 || y >= res.y || y < 0) {
        return;
    }
    uint index = y * res.x + x;
    pixel[index] = val;
}

void Visualizer::draw(ocarina::float4 *data) const noexcept {
    if (!show_) { return; }
    line_bresenham(make_float2(0), make_float2(500, 500), make_float4(1,0,0,1), data);
}

bool Visualizer::render_UI(ocarina::Widgets *widgets) noexcept {
    bool ret = widgets->use_folding_header("Visualizer", [&] {
#define visualize_macro(name)                              \
    if (widgets->radio_button(#name, state_ == E##name)) { \
        state_ = E##name;                                  \
    }
        visualize_macro(Off);
        visualize_macro(Ray);
        visualize_macro(Normal);
#undef visualize_macro
        render_sub_UI(widgets);
    });
    return ret;
}

void Visualizer::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    widgets->button_click("clear", [&] {
        clear();
    });
    widgets->check_box("show", &show_);
}

void Visualizer::clear() noexcept {
}

}// namespace vision

VS_REGISTER_HOTFIX(vision, Visualizer)
VS_REGISTER_CURRENT_PATH(1, "vision-base.dll")