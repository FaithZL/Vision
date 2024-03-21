//
// Created by Zero on 11/10/2022.
//

#include "sensor.h"
#include "base/mgr/pipeline.h"
#include "GUI/window.h"

namespace vision {
using namespace ocarina;

Sensor::Sensor(const SensorDesc &desc)
    : Node(desc),
      _filter(scene().load<Filter>(desc.filter_desc)),
      _film(scene().load<Film>(desc.film_desc)),
      _medium_id(desc.medium.id) {
    if (!scene().has_medium()) {
        return;
    }
    if (desc.contains("medium")) {
        _medium.name = desc["medium"].as_string();
    } else {
        _medium = scene().global_medium();
    }
    auto &mediums = scene().mediums();
    auto iter = std::find_if(mediums.begin(), mediums.end(), [&](const SP<Medium> &medium) {
        return _medium.name == medium->name();
    });
    if (iter != mediums.end()) {
        _medium.object = *iter;
        _medium_id = _medium.object->index();
    }
}

bool Sensor::render_UI(ocarina::Widgets *widgets) noexcept {
    bool open = widgets->use_folding_header(
        ocarina::format("{} camera", impl_type().data()),
        [&] {
            widgets->text("type: %s", impl_type().data());
            render_sub_UI(widgets);
            _filter->render_UI(widgets);
            _film->tone_mapper()->render_UI(widgets);
        });
    return open;
}

void Sensor::prepare() noexcept {
    _filter->prepare();
    _film->prepare();
}

}// namespace vision