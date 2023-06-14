//
// Created by Zero on 11/10/2022.
//

#include "sensor.h"
#include "base/mgr/pipeline.h"

namespace vision {
using namespace ocarina;

Sensor::Sensor(const SensorDesc &desc)
    : Node(desc),
      _filter(scene().load<Filter>(desc.filter_desc)),
      _radiance_film(scene().load<Film>(desc.film_desc)),
      _medium(desc.medium.id) {}

void Sensor::prepare() noexcept {
    _filter->prepare();
    _radiance_film->prepare();
}

}// namespace vision