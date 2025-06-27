//
// Created by ling.zhu on 2025/6/27.
//

#pragma once

#include "base/node.h"
#include "base/sample.h"
#include "math/transform.h"
#include "filter.h"
#include "hotfix/hotfix.h"

namespace vision {
using namespace ocarina;

class RayGenerator : public Node, public Encodable, public Observer {
public:
    using Desc = RayGeneratorDesc;

private:
    uint2 resolution_;

public:
    RayGenerator() = default;
    explicit RayGenerator(const RayGeneratorDesc &desc)
        : Node(desc) {}

    [[nodiscard]] virtual uint2 launch_dim() const noexcept { return resolution_; }
    void set_resolution(uint2 res) noexcept { resolution_ = res; }
    void update_resolution(uint2 res) noexcept {
        set_resolution(res);
    }
    virtual void on_resize(uint2 res) noexcept {}
    [[nodiscard]] uint pixel_num() const noexcept { return resolution_.x * resolution_.y; }
};

}// namespace vision