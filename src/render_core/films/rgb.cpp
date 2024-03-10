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
    RegistrableManaged<float4> _rt_buffer;
    RegistrableManaged<float4> _output_buffer;
    Shader<void(Buffer<float4>)> _accumulate;
    bool _gamma{true};

public:
    explicit RGBFilm(const FilmDesc &desc)
        : Film(desc),
          _rt_buffer(pipeline()->bindless_array()),
          _output_buffer(pipeline()->bindless_array()),
          _gamma(desc["gamma"].as_bool(true)) {}

    OC_SERIALIZABLE_FUNC(Film, _rt_buffer, _output_buffer)
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }

    void compile() noexcept override {

    }
    void prepare() noexcept override {
        auto prepare = [&](RegistrableManaged<float4> &managed, const string &desc = "") noexcept {
            managed.reset_all(device(), pixel_num(), desc);
            managed.reset_immediately();
            managed.register_self();
        };
        prepare(_rt_buffer, "RGBFilm::_rt_buffer");
        prepare(_output_buffer, "RGBFilm::_output_buffer");
    }
    void add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept override {
        Float a = 1.f / (frame_index + 1);
        Uint index = dispatch_id(pixel);
        val = Env::instance().zero_if_nan_inf(val);
        if (_accumulation.hv()) {
            Float4 accum_prev = _rt_buffer.read(index);
            val = lerp(make_float4(a), accum_prev, val);
        }
        _rt_buffer.write(index, val);
        val = _tone_mapper->apply(val);
        if (_gamma) {
            val = linear_to_srgb(val);
        }
        val.w = 1.f;
        _output_buffer.write(index, val);
    }
    [[nodiscard]] CommandList accumulate() const noexcept override {
        return {};
    }
    [[nodiscard]] CommandList gamma_correct() const noexcept override {
        return {};
    }
    [[nodiscard]] CommandList tone_mapping() const noexcept override {
        return {};
    }
    [[nodiscard]] const RegistrableManaged<float4> &output_buffer() const noexcept override { return _output_buffer; }
    [[nodiscard]] RegistrableManaged<float4> &output_buffer() noexcept override { return _output_buffer; }
    [[nodiscard]] const RegistrableManaged<float4> &rt_buffer() const noexcept override { return _rt_buffer; }
    [[nodiscard]] RegistrableManaged<float4> &rt_buffer() noexcept override { return _rt_buffer; }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)