//
// Created by Zero on 11/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "rhi/common.h"
#include "dsl/common.h"
#include "node.h"
#include "tonemapping.h"

namespace vision {
using namespace ocarina;

//"film": {
//    "type": "rgb",
//    "param": {
//        "resolution": [
//            1280,
//            720
//            ],
//         "fb_state": 0
//    }
//}
class Film : public Node {
public:
    using Desc = FilmDesc;

private:
    uint2 _resolution;


public:
    explicit Film(const FilmDesc &desc)
        : Node(desc),
          _resolution(desc["resolution"].as_uint2(make_uint2(768u))) {}
    [[nodiscard]] uint pixel_num() const noexcept { return _resolution.x * _resolution.y; }
    [[nodiscard]] Uint pixel_index(Uint2 pixel) const noexcept { return pixel.y * _resolution.x + pixel.x; }
    void set_resolution(uint2 res) noexcept { _resolution = res; }
    [[nodiscard]] uint2 resolution() const noexcept { return _resolution; }
    virtual void add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept = 0;
    virtual void add_sample(const Uint2 &pixel, const Float3 &val, const Uint &frame_index) noexcept {
        add_sample(pixel, make_float4(val, 1.f), frame_index);
    }
    virtual void copy_to(void *host_ptr) const noexcept = 0;
};
}// namespace vision