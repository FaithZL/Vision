//
// Created by Zero on 2023/5/30.
//

#include "radiance_collector.h"
#include "base/mgr/scene.h"

namespace vision {

RadianceCollector::RadianceCollector(const vision::RadianceCollectorDesc &desc)
    : Node(desc),
      tone_mapper_(desc.tone_mapper),
      resolution_(desc["resolution"].as_uint2(make_uint2(1280, 720))),
      screen_window_(make_float2(-1.f), make_float2(1.f)),
      exposure_(desc["exposure"].as_float(1.f)),
      accumulation_(desc["accumulation"].as_uint(1)) {
    update_screen_window();
}

void RadianceCollector::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    changed_ |= widgets->drag_float("exposure", addressof(exposure_.hv()), 0.01, 0.f);
}

Float4 RadianceCollector::apply_exposure(const ocarina::Float4 &input) const noexcept {
    return 1.f - exp(-input * *exposure_);
}

void RadianceCollector::update_runtime_object(const IObjectConstructor *constructor) noexcept {
    std::tuple tp = {addressof(tone_mapper_.impl())};
    HotfixSystem::replace_objects(constructor, tp);
}

void RadianceCollector::update_screen_window() noexcept {
    float ratio = resolution_.x * 1.f / resolution_.y;
    if (ratio > 1.f) {
        screen_window_.lower.x = -ratio;
        screen_window_.upper.x = ratio;
    } else {
        screen_window_.lower.y = -1.f / ratio;
        screen_window_.upper.y = 1.f / ratio;
    }
}

}// namespace vision