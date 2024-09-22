//
// Created by Zero on 2024/9/21.
//

#include "visualizer.h"
#include "base/sensor/camera.h"

namespace vision {


void Visualizer::init() noexcept {
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
    widgets->button_click("clear", [&]{
        clear();
    });
}

void Visualizer::draw(const ocarina::float4 *data, ocarina::uint2 res) const noexcept {
}

void Visualizer::clear() noexcept {
}

}// namespace vision

VS_REGISTER_HOTFIX(vision, Visualizer)
VS_REGISTER_CURRENT_PATH(0, "vision-visualize.dll")