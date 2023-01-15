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
    Buffer<float4> _radiance;
    Buffer<float4> _frame;
public:
    explicit RGBFilm(const FilmDesc &desc) : Film(desc) {}
    void prepare() noexcept override {
        _radiance = device().create_buffer<float4>(pixel_num());
        _frame = device().create_buffer<float4>(pixel_num());
        _radiance.clear_immediately();
        _frame.clear_immediately();
    }
    void add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept override {
        Float a = 1.f / (frame_index + 1);
        Uint index = pixel_index(pixel);
        Float4 accum_prev = _radiance.read(index);
        val = lerp(make_float4(a), accum_prev, val);
        _radiance.write(index, val);
        _frame.write(index, linear_to_srgb(val));
    }
    void copy_to(void *host_ptr) const noexcept override {
        _frame.download_immediately(host_ptr);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)