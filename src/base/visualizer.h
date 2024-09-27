//
// Created by Zero on 2024/9/21.
//

#pragma once

#include "core/stl.h"
#include "math/basic_types.h"
#include "base/node.h"
#include "dsl/dsl.h"
#include "UI/GUI.h"
#include "hotfix/hotfix.h"
#include "sensor/camera.h"

namespace vision {
struct LineSegment {
    float3 start;
    float3 end;
};
}// namespace vision

//clang-format off
OC_STRUCT(vision, LineSegment, start, end){};
//clang-format on

namespace vision {

using namespace ocarina;

class Visualizer : public Context, public GUI, public RuntimeObject {
public:
    enum State {
        EOff,
        ERay,
        ENormal
    };

private:
    State state_{EOff};
    bool show_{false};
    ManagedList<LineSegment> line_segments_;

public:
    Visualizer() = default;
    HOTFIX_VIRTUAL void init() noexcept;
    HOTFIX_VIRTUAL void draw(const float4 *data) const noexcept;
    HOTFIX_VIRTUAL void clear() noexcept;
    OC_MAKE_MEMBER_GETTER(show, )
    [[nodiscard]] Camera *camera() const noexcept;
    [[nodiscard]] uint2 resolution() const noexcept;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
};

}// namespace vision
