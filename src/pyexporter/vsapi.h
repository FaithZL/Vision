//
// Created by Zero on 2024/11/4.
//

#pragma once

#include "ext/pybind11/include/pybind11/pybind11.h"
#include "ext/pybind11/include/pybind11/stl.h"
#include "core/stl.h"
#include "rhi/device.h"
#include "base/mgr/scene.h"

namespace py = pybind11;
using namespace ocarina;

struct VisionPyExporter {
    py::module module;
};

namespace vision::inline python {
using namespace ocarina;
struct Context {
    [[nodiscard]] static Context &instance() noexcept;
    Context() {
        OC_INFO("ocapi load!");
    }
    ~Context() {
        OC_INFO("ocapi unload!");
    }
};
}// namespace vision::inline python