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
public:
    virtual bool render_UI(ocarina::Widgets *widgets) noexcept { return true; }
    virtual bool render_sub_UI(ocarina::Widgets *widgets) noexcept { return true; }
    virtual ~GUI() {}
};

}// namespace vision