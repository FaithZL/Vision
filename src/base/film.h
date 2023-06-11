//
// Created by Zero on 11/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "rhi/common.h"
#include "dsl/common.h"
#include "node.h"
#include "tonemapper.h"

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
class Film : public Node, public Serializable<float> {
public:
    using Desc = FilmDesc;

protected:
    uint2 _resolution;
    ToneMapper *_tone_mapper{};

public:
    explicit Film(const FilmDesc &desc);
    OC_SERIALIZABLE_FUNC(*_tone_mapper)
    [[nodiscard]] uint pixel_num() const noexcept { return _resolution.x * _resolution.y; }
    [[nodiscard]] Uint pixel_index(Uint2 pixel) const noexcept { return pixel.y * _resolution.x + pixel.x; }
    void set_resolution(uint2 res) noexcept { _resolution = res; }
    [[nodiscard]] const ToneMapper *tone_mapper() const noexcept { return _tone_mapper; }
    [[nodiscard]] ToneMapper *tone_mapper() noexcept { return _tone_mapper; }
    [[nodiscard]] uint2 resolution() const noexcept { return _resolution; }
    virtual void add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept = 0;
    virtual void add_sample(const Uint2 &pixel, const Float3 &val, const Uint &frame_index) noexcept {
        add_sample(pixel, make_float4(val, 1.f), frame_index);
    }
    virtual void copy_tone_mapped_buffer(void *dst_ptr) const noexcept = 0;
    virtual void copy_raw_buffer(void *dst_ptr) const noexcept = 0;
};
}// namespace vision