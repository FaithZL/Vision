//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "core/stl.h"

namespace vision {
class SceneNode {
private:
    ocarina::string _name;

public:
    SceneNode() = default;
    SceneNode(const ocarina::string &name) : _name(name) {}
    virtual ~SceneNode() = default;
    [[nodiscard]] ocarina::string name() const noexcept { return _name; }
};
}
