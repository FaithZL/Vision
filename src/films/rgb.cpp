//
// Created by Zero on 13/10/2022.
//

#include "base/film.h"
#include "core/render_pipeline.h"

namespace vision {
using namespace ocarina;

class RGBFilm : public Film {
private:
    Buffer<float4> _radiance_buffer;

public:
    explicit RGBFilm(const FilmDesc &desc) : Film(desc) {}
    void prepare(RenderPipeline *rp) noexcept override {
        _radiance_buffer = rp->device().create_buffer<float4>(pixel_num());
    }
    void add_sample(Uint2 pixel, Float4 val, Uint frame_index) noexcept override {
        Uint index = pixel_index(pixel);
        _radiance_buffer.write(index, val);
    }
    void copy_to(void *host_ptr) const noexcept override {
        _radiance_buffer.download_immediately(host_ptr);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)