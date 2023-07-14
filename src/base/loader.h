//
// Created by Zero on 2023/7/14.
//

#pragma once

#include <utility>
#include "core/stl.h"
#include "mgr/scene.h"

namespace vision {

class Loader {
public:
    static Loader *create(const fs::path &fn);
    [[nodiscard]] virtual Scene load() = 0;
};

}// namespace vision