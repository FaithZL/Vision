//
// Created by Zero on 13/10/2022.
//

#include "base/film.h"
#include "core/render_pipeline.h"
#include "math/base.h"

namespace vision {
using namespace ocarina;

class RGBFilm : public Film {
private:
    Image _radiance;
    Image _frame;
public:
    explicit RGBFilm(const FilmDesc &desc) : Film(desc) {}
    void prepare(RenderPipeline *rp) noexcept override {
        _radiance = rp->device().create_image(resolution(), PixelStorage::FLOAT4);
        _frame = rp->device().create_image(resolution(), PixelStorage::FLOAT4);
    }
    void add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept override {
        Float a = 1.f / (frame_index + 1);
        Float4 accum_prev = _radiance.read<float4>(pixel);
        val = lerp(make_float4(a), accum_prev, val);
        _radiance.write(pixel, val);
        _frame.write(pixel, linear_to_srgb(val));
    }
    void copy_to(void *host_ptr) const noexcept override {
        _frame.download_immediately(host_ptr);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)