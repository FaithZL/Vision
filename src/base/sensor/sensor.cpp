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
      filter_(desc.filter_desc),
      film_(Node::create_shared<Film>(desc.film_desc)),
      medium_id_(desc.medium.id) {
    if (!scene().has_medium()) {
        return;
    }
    TObject<Medium> medium_;
    if (desc.contains("medium")) {
        medium_.name = desc["medium"].as_string();
    } else {
        medium_ = scene().global_medium();
    }
    auto &mediums = scene().mediums();
    auto iter = std::find_if(mediums.begin(), mediums.end(), [&](const SP<Medium> &medium) {
        return medium_.name == medium->name();
    });
    if (iter != mediums.end()) {
        medium_.init(*iter);
        medium_id_ = medium_->index();
    }
}

void Sensor::update_runtime_object(const vision::IObjectConstructor *constructor) noexcept {
    std::tuple tp = {addressof(filter_.impl()),
        addressof(film_)};
    HotfixSystem::replace_objects(constructor, tp);
}

void Sensor::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    if (medium_id_.hv() != InvalidUI32) {
        auto medium = MediumRegistry::instance().elements()[medium_id_.hv()];
        medium->render_UI(widgets);
    }
}

bool Sensor::render_UI(ocarina::Widgets *widgets) noexcept {
    bool open = widgets->use_folding_header(
        ocarina::format("{} camera", impl_type().data()),
        [&] {
            widgets->text("type: %s", impl_type().data());
            render_sub_UI(widgets);
        });
    filter_.render_UI(widgets);
    film_->render_UI(widgets);
    return open;
}

void Sensor::prepare() noexcept {
    filter_->prepare();
    film_->prepare();
}

}// namespace vision