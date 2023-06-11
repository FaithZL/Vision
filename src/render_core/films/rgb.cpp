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
    using _serial_ty = Film;

private:
    RegistrableManaged<float4> _radiance;
    RegistrableManaged<float4> _frame;

public:
    explicit RGBFilm(const FilmDesc &desc)
        : Film(desc),
          _radiance(render_pipeline()->resource_array()),
          _frame(render_pipeline()->resource_array()) {}

    OC_SERIALIZABLE_FUNC(_radiance, _frame)

    void prepare(RegistrableManaged<float4> &managed) noexcept {
        managed.device() = device().create_buffer<float4>(pixel_num());
        managed.host().resize(pixel_num());
        managed.clear_immediately();
        managed.register_self();
    }

    void prepare() noexcept override {
        prepare(_radiance);
        prepare(_frame);
    }
    void add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept override {
        Float a = 1.f / (frame_index + 1);
        Uint index = pixel_index(pixel);
        Float4 accum_prev = _radiance.read(index);
        val = lerp(make_float4(a), accum_prev, val);
        _radiance.write(index, val);
        val = linear_to_srgb(_tone_mapper->apply(val));
        val.w = 1.f;
        _frame.write(index, val);
    }
    void copy_tone_mapped_buffer(void *dst_ptr) const noexcept override {
        _frame.device().download_immediately(dst_ptr);
    }
    [[nodiscard]] const RegistrableManaged<float4> &tone_mapped_buffer() const noexcept override { return _frame; }
    [[nodiscard]] RegistrableManaged<float4> &tone_mapped_buffer() noexcept override { return _frame; }
    [[nodiscard]] const RegistrableManaged<float4> &original_buffer() const noexcept override { return _radiance; }
    [[nodiscard]] RegistrableManaged<float4> &original_buffer() noexcept override { return _radiance; }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)