//
// Created by Zero on 2023/6/14.
//

#include "global.h"

namespace vision {

Global *Global::s_global = nullptr;

Global &Global::instance() {
    if (s_global == nullptr) {
        s_global = new Global();
    }
    return *s_global;
}

void Global::destroy_instance() {
    if (s_global) {
        delete s_global;
        s_global = nullptr;
    }
}
}// namespace vision