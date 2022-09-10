//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "ocarina/src/core/stl.h"

#include "scene_desc.h"

using namespace ocarina;

namespace vision {

class SceneParser {
private:
    DataWrap _data;

public:
    SceneParser() = default;
    explicit SceneParser(const fs::path &fn) { load(fn); }
    void load(const fs::path &fn);
    unique_ptr<SceneDesc> parse() const noexcept;
};
}// namespace vision