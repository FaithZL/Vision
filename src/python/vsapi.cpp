//
// Created by Zero on 2024/11/4.
//

#include "vsapi.h"

Context &Context::instance() noexcept {
    static Context ret;
    return ret;
}

PYBIND11_MODULE(vsapi, m) {
    m.def("test", [&](int a, int b) {
        return a + b;
    });
}