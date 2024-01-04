//
// Created by Zero on 13/10/2022.
//

#include "base/sensor/film.h"
#include "base/mgr/pipeline.h"
#include "math/base.h"

namespace vision {
using namespace ocarina;

class RGBFilm : public Film {
private:
    RegistrableManaged<float4> _radiance;
    RegistrableManaged<float4> _frame;
    bool _gamma{true};

public:
    explicit RGBFilm(const FilmDesc &desc)
        : Film(desc),
          _radiance(pipeline()->bindless_array()),
          _frame(pipeline()->bindless_array()),
          _gamma(desc["gamma"].as_bool(true)) {}

    OC_SERIALIZABLE_FUNC(Film, _radiance, _frame)
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void prepare() noexcept override {
        auto prepare = [&](RegistrableManaged<float4> &managed, const string &desc = "") noexcept {
            managed.reset_all(device(), pixel_num(), desc);
            managed.reset_immediately();
            managed.register_self();
        };
        prepare(_radiance, "RGBFilm::_radiance");
        prepare(_frame, "RGBFilm::_frame");
    }
    void add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept override {
        Float a = 1.f / (frame_index + 1);
        Uint index = pixel_index(pixel);
        if (_accumulation.hv()) {
            Float4 accum_prev = _radiance.read(index);
            val = lerp(make_float4(a), accum_prev, val);
        }
        _radiance.write(index, val);
        val = _tone_mapper->apply(val);
        if (_gamma) {
            val = linear_to_srgb(val);
        }
        val.w = 1.f;
        _frame.write(index, val);
    }
    [[nodiscard]] const RegistrableManaged<float4> &tone_mapped_buffer() const noexcept override { return _frame; }
    [[nodiscard]] RegistrableManaged<float4> &tone_mapped_buffer() noexcept override { return _frame; }
    [[nodiscard]] const RegistrableManaged<float4> &original_buffer() const noexcept override { return _radiance; }
    [[nodiscard]] RegistrableManaged<float4> &original_buffer() noexcept override { return _radiance; }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)