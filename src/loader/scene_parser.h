//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "ocarina/src/core/stl.h"
#include "ext/nlohmann/json.hpp"

using namespace ocarina;

namespace vision {

class SceneConfig;
using DataWrap = nlohmann::json;

class SceneParser {
private:
    DataWrap _data;

public:
    void load(const fs::path &fn) {

    }

    shared_ptr<SceneConfig> parse() const noexcept;
};
}// namespace vision