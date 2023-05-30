//
// Created by Zero on 13/10/2022.
//

#include "base/film.h"
#include "base/mgr/render_pipeline.h"
#include "math/base.h"

namespace vision {
using namespace ocarina;

class RGBFilm : public Film {
private:
    BufferWrapper<float4> _radiance;
    BufferWrapper<float4> _frame;

public:
    explicit RGBFilm(const FilmDesc &desc)
        : Film(desc),
          _radiance(render_pipeline()->resource_array()),
          _frame(render_pipeline()->resource_array()) {}
    void prepare() noexcept override {
        _radiance.super() = device().create_buffer<float4>(pixel_num());
        _frame.super() = device().create_buffer<float4>(pixel_num());
        _radiance.clear_immediately();
        _frame.clear_immediately();
        _radiance.register_self();
        _frame.register_self();
    }
    void add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept override {
        Float a = 1.f / (frame_index + 1);
        Uint index = pixel_index(pixel);
        Float4 accum_prev = _radiance.read(index);
        val = lerp(make_float4(a), accum_prev, val);
        _radiance.write(index, val);
        val = linear_to_srgb(_tone_mapping->apply(val));
        val.w = 1.f;
        _frame.write(index, val);
    }
    void copy_to(void *host_ptr) const noexcept override {
        _frame.download_immediately(host_ptr);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)