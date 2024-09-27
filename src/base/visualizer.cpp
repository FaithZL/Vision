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

void Visualizer::draw(const ocarina::float4 *data) const noexcept {
    if (!show_) { return; }
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
}

void Visualizer::clear() noexcept {
}

}// namespace vision

VS_REGISTER_HOTFIX(vision, Visualizer)
VS_REGISTER_CURRENT_PATH(1, "vision-base.dll")