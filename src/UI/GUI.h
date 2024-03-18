//
// Created by Zero on 2024/3/17.
//

#pragma once

#include "GUI/window.h"

namespace vision {

using namespace ocarina;

class GUI {
public:
    virtual bool render_UI(Widgets *widgets) noexcept { return true; }
    virtual ~GUI() {}
};

}// namespace vision