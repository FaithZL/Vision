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
      _medium_id(desc.medium.id) {
    if (!scene().has_medium()) {
        return ;
    }
    _medium.name = desc["medium"].as_string();
    auto &mediums = scene().mediums();
    auto iter = std::find_if(mediums.begin(), mediums.end(), [&](const SP<Medium> &medium) {
        return _medium.name == medium->name();
    });
    if (iter != mediums.end()) {
        _medium.object = *iter;
        _medium_id = _medium.object->index();
    }
}

void Sensor::prepare() noexcept {
    _filter->prepare();
    _radiance_film->prepare();
}

}// namespace vision