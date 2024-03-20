//
// Created by Zero on 2024/3/20.
//

#pragma once

#include "GUI.h"
#include "dsl/dsl.h"

namespace vision {
template<typename T>
class PolymorphicGUI : public GUI, public ocarina::Polymorphic<T> {
public:
    using Super = ocarina::Polymorphic<T>;

public:
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        Super::for_each_representative([&](auto elm) {

        });
        return true;
    }

    void reset_status() noexcept override {
        _changed = false;
        Super::for_each_instance([&](auto elm) {
            UI::reset_status(elm);
        });
    }

    [[nodiscard]] bool has_changed() noexcept override {
        bool ret = _changed;
        Super::for_each_instance([&](auto elm) {
            ret |= UI::has_changed(elm);
        });
        return ret;
    }
};
}// namespace vision
