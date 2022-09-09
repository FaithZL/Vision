//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "ocarina/src/core/stl.h"
#include "ext/nlohmann/json.hpp"
#include "scene_config.h"

using namespace ocarina;

namespace vision {

using DataWrap = nlohmann::json;

class SceneParser {
private:
    DataWrap _data;

public:
    SceneParser() = default;
    SceneParser(const fs::path &fn) { load(fn); }
    void load(const fs::path &fn);
    unique_ptr<SceneConfig> parse() const noexcept;
};
}// namespace vision