//
// Created by ling.zhu on 2025/4/16.
//

#include "medium.h"
#include "base/mgr/registries.h"

namespace vision {

bool Medium::render_UI(ocarina::Widgets *widgets) noexcept {
    string label = format("{} {} medium: {}, type_index: {}",
                          index_, impl_type().data(),
                          name_.c_str(), MediumRegistry::instance().elements().type_index(this));
    bool open = widgets->use_tree(label, [&] {
        render_sub_UI(widgets);
    });
    return open;
}

}// namespace vision