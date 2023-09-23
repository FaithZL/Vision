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
      _screen_window(make_float2(-1.f),make_float2(1.f)) {}

}// namespace vision