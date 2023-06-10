//
// Created by Zero on 2023/5/30.
//

#include "film.h"
#include "mgr/scene.h"

namespace vision {

Film::Film(const vision::FilmDesc &desc)
    : Node(desc),
      _tone_mapper(desc.scene->load<ToneMapper>(desc.tone_mapper)),
      _resolution(desc["resolution"].as_uint2(make_uint2(768u))) {}

}// namespace vision