//
// Created by ling.zhu on 2025/4/16.
//

#include "medium.h"
#include "base/mgr/registries.h"
#include "base/shape.h"

namespace vision {

void Medium::restore(vision::RuntimeObject *old_obj) noexcept {
    Node::restore(old_obj);
    VS_HOTFIX_MOVE_ATTRS(index_, scale_)
    for (const auto &item : old_obj_->inside_ref_instances) {
        auto sp = item.lock();
        sp->set_inside(shared_from_this());
        sp->set_inside_name(name());
    }
    for (const auto &item : old_obj_->outside_ref_instances) {
        auto sp = item.lock();
        sp->set_outside(shared_from_this());
        sp->set_outside_name(name());
    }
}

void Medium::add_inside_reference(SP<ShapeInstance> shape_instance) noexcept {
    inside_ref_instances.push_back(ocarina::move(shape_instance));
}

void Medium::add_outside_reference(SP<ShapeInstance> shape_instance) noexcept {
    outside_ref_instances.push_back(ocarina::move(shape_instance));
}

bool Medium::render_UI(ocarina::Widgets *widgets) noexcept {
    string label = format("{} {} medium: {}, type_index: {}",
                          index_, impl_type().data(),
                          name_.c_str(), MediumRegistry::instance().elements().type_index(this));
    bool open = widgets->use_tree(label, [&] {
        render_sub_UI(widgets);
    });
    return open;
}

void Medium::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    changed_ |= widgets->drag_float("scale", addressof(scale_.hv()), 0.05f, 0);
}

}// namespace vision