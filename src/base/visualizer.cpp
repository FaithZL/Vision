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
    line_segments_.set_bindless_array(pipeline()->bindless_array());
    line_segments_.set_list(device().create_list<LineSegment>(10000, "line segments"));
    line_segments_.register_self();
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
//    $info_with_location("add ray {} {} {}    {} {} {}", p0, p1);
    LineSegmentVar line_segment;
    line_segment.start = p0;
    line_segment.end = p1;
    auto p2 = camera()->raster_coord(line_segment.start);
    auto p3 = camera()->raster_coord(line_segment.end);
    line_segments_.push_back(line_segment);
}

void Visualizer::draw(ocarina::float4 *data) const noexcept {
    if (!show_) { return; }
    static vector<LineSegment> host;
    host.resize(line_segments_.capacity());
    stream() << line_segments_.storage_segment().download(host.data(), false);
    uint count = line_segments_.host_count();

    for (int i = 0; i < count; ++i) {
        LineSegment ls = host[i];
        auto p0 = camera()->raster_coord(ls.start);
        auto p1 = camera()->raster_coord(ls.end);

        p0.x = ocarina::isnan(p0.x) ? resolution().x / 2 : p0.x;
        p0.y = ocarina::isnan(p0.y) ? resolution().y / 2 : p0.y;
        p1.x = ocarina::isnan(p1.x) ? resolution().x / 2 : p1.x;
        p1.y = ocarina::isnan(p1.y) ? resolution().y / 2 : p1.y;
        safe_line_bresenham(p0.xy(), p1.xy(), [&](int x, int y) {
            write(x, y, make_float4(color_, 1), data);
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
        cout << line_segments_.bindless_array() << endl;
//        clear();
    });
    widgets->check_box("show", &show_);
    widgets->color_edit("color", &color_);
}

}// namespace vision

VS_REGISTER_HOTFIX(vision, Visualizer)
VS_REGISTER_CURRENT_PATH(1, "vision-base.dll")