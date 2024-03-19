//
// Created by Zero on 2024/3/17.
//

#pragma once

#include "GUI/decl.h"

namespace ocarina {
class Widgets;
}

namespace vision {

class GUI {
protected:
    bool _changed{false};

public:
    virtual void reset_status() noexcept { _changed = false; }
    [[nodiscard]] virtual bool has_changed() noexcept { return _changed; }

    /**
     * @param widgets
     * @return Status of the widget switch
     */
    virtual bool render_UI(ocarina::Widgets *widgets) noexcept { return true; }

    /**
     * @param widgets
     * @return Whether any data has been updated
     */
    virtual bool render_sub_UI(ocarina::Widgets *widgets) noexcept { return true; }
    virtual ~GUI() = default;
};

}// namespace vision