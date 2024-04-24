//
// Created by Zero on 2023/5/30.
//

#include "film.h"
#include "base/mgr/scene.h"

namespace vision {

Film::Film(const vision::FilmDesc &desc)
    : Node(desc),
      tone_mapper_(Node::load<ToneMapper>(desc.tone_mapper)),
      resolution_(desc["resolution"].as_uint2(make_uint2(1280, 720))),
      screen_window_(make_float2(-1.f), make_float2(1.f)),
      accumulation_(desc["accumulation"].as_uint(1)){
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