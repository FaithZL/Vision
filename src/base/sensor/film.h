//
// Created by Zero on 11/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "rhi/common.h"
#include "dsl/dsl.h"
#include "base/node.h"
#include "tonemapper.h"
#include "math/box.h"

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
    Box2f _screen_window;
    Serial<uint> _accumulation;
    SP<ToneMapper> _tone_mapper{};

public:
    explicit Film(const FilmDesc &desc);
    OC_SERIALIZABLE_FUNC(Serializable<float>, _accumulation, *_tone_mapper)
    virtual void compile() noexcept = 0;
    [[nodiscard]] uint pixel_num() const noexcept { return _resolution.x * _resolution.y; }
    [[nodiscard]] Box2f screen_window() const noexcept { return _screen_window; }
    [[nodiscard]] bool enable_accumulation() const noexcept { return _accumulation.hv(); }
    void set_resolution(uint2 res) noexcept { _resolution = res; }
    [[nodiscard]] auto tone_mapper() const noexcept { return _tone_mapper; }
    [[nodiscard]] auto tone_mapper() noexcept { return _tone_mapper; }
    [[nodiscard]] uint2 resolution() const noexcept { return _resolution; }
    virtual void add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept = 0;
    virtual void add_sample(const Uint2 &pixel, const Float3 &val, const Uint &frame_index) noexcept {
        add_sample(pixel, make_float4(val, 1.f), frame_index);
    }
    [[nodiscard]] virtual CommandList accumulate(BufferView<float4> input,BufferView<float4> output,
                                                 uint frame_index) const noexcept = 0;
    [[nodiscard]] virtual CommandList tone_mapping(BufferView<float4> input,
                                                   BufferView<float4> output) const noexcept = 0;
    [[nodiscard]] virtual CommandList gamma_correct(BufferView<float4> input,
                                                    BufferView<float4> output) const noexcept = 0;
    [[nodiscard]] virtual const RegistrableManaged<float4> &output_buffer() const noexcept = 0;
    [[nodiscard]] virtual RegistrableManaged<float4> &output_buffer() noexcept = 0;
    [[nodiscard]] virtual const RegistrableManaged<float4> &accumulation_buffer() const noexcept = 0;
    [[nodiscard]] virtual RegistrableManaged<float4> &accumulation_buffer() noexcept = 0;
    [[nodiscard]] virtual const RegistrableManaged<float4> &rt_buffer() const noexcept = 0;
    [[nodiscard]] virtual RegistrableManaged<float4> &rt_buffer() noexcept = 0;
};
}// namespace vision