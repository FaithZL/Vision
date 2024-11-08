//
// Created by Zero on 2024/11/4.
//

#include "vsapi.h"

namespace vision::inline python {
Context &Context::instance() noexcept {
    static Context ret;
    return ret;
}
}// namespace vision::inline python

PYBIND11_MODULE(vsapi, m) {
    m.def("test", [&](int a, int b) {
        return a + b;
    });
}