//
// Created by Zero on 2023/5/30.
//

#include "film.h"
#include "base/mgr/scene.h"

namespace vision {

Film::Film(const vision::FilmDesc &desc)
    : Node(desc),
      _tone_mapper(scene().load<ToneMapper>(desc.tone_mapper)),
      _resolution(desc["resolution"].as_uint2(make_uint2(1280, 720))),
      _screen_window(make_float2(-1.f), make_float2(1.f)),
      _accumulation(desc["accumulation"].as_uint(1)){
    float ratio = _resolution.x * 1.f / _resolution.y;
    if (ratio > 1.f) {
        _screen_window.lower.x = -ratio;
        _screen_window.upper.x = ratio;
    } else {
        _screen_window.lower.y = -1.f / ratio;
        _screen_window.upper.y = 1.f / ratio;
    }
}

}// namespace vision