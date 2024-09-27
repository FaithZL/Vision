//
// Created by Zero on 2024/9/21.
//

#include "visualizer.h"
#include "base/sensor/camera.h"
#include "mgr/pipeline.h"
#include "math/util.h"

namespace vision {

Camera *Visualizer::camera() const noexcept {
    return scene().camera().get();
}

uint2 Visualizer::resolution() const noexcept {
    return pipeline()->resolution();
}

void Visualizer::init() noexcept {
    line_segments_ = device().create_managed_list<LineSegment>(10000, "line segments");
    clear();
}

void Visualizer::clear() noexcept {
    line_segments_.clear_immediately();
}

void Visualizer::write(int x, int y, ocarina::float4 val, ocarina::float4 *pixel) const noexcept {
    uint2 res = resolution();
    if (x >= res.x || x < 0 || y >= res.y || y < 0) {
        return;
    }
    uint index = y * res.x + x;
    pixel[index] = val;
}

void Visualizer::add_line_segment(const Float3 &p0, const Float3 &p1) noexcept {
    if (state_ != ERay) { return; }
    $info_with_location("{} {} {}   {} {} {}", p0, p1);
    LineSegmentVar line_segment;
    line_segment.start = p0;
    line_segment.end = p1;
    auto p2 = camera()->raster_coord(line_segment.start);
    auto p3 = camera()->raster_coord(line_segment.end);
    $condition_info_with_location("{} {} {}      {} {} {}  ---------", p2, p3);
    line_segments_.device_list().push_back(line_segment);
}

void Visualizer::draw(ocarina::float4 *data) const noexcept {
    if (!show_) { return; }
    line_segments_.download_immediately();
    uint count = line_segments_.host_count();

    for (int i = 0; i < count; ++i) {
        LineSegment ls = line_segments_[i];
        auto p0 = camera()->raster_coord(ls.start);
        auto p1 = camera()->raster_coord(ls.end);
        safe_line_bresenham(p0.xy(), p1.xy(), [&](int x, int y) {
            write(x, y, make_float4(1, 0, 0, 1), data);
        });
    }
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

}// namespace vision

VS_REGISTER_HOTFIX(vision, Visualizer)
VS_REGISTER_CURRENT_PATH(1, "vision-base.dll")