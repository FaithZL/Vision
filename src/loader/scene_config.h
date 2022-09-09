//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"

namespace vision {
using namespace ocarina;
struct SceneConfig {
public:


    static unique_ptr<SceneConfig> from_json(const fs::path &path);
};

}// namespace vision