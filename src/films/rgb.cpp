//
// Created by Zero on 13/10/2022.
//

#include "base/film.h"
#include "core/render_pipeline.h"

namespace vision {
using namespace ocarina;

class RGBFilm : public Film {
public:
    using Desc = FilmDesc;

private:
    Buffer<float4> _radiance_buffer;

public:
    explicit RGBFilm(const FilmDesc *desc) : Film(desc) {}
    void prepare(RenderPipeline *rp) noexcept override {
        _radiance_buffer = rp->device().create_buffer<float4>(pixel_num());
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)