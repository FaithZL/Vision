//
// Created by Zero on 13/10/2022.
//

#include "base/sensor/film.h"
#include "base/mgr/pipeline.h"
#include "math/base.h"

namespace vision {
using namespace ocarina;

/// temporary solution
class RGBFilm : public Film {
private:
    RegistrableManaged<float4> _rt_buffer;
    RegistrableManaged<float4> _accumulation_buffer;
    RegistrableManaged<float4> _output_buffer;

    Shader<void(Buffer<float4>, Buffer<float4>, uint)> _accumulate;
    Shader<void(Buffer<float4>, Buffer<float4>)> _tone_mapping;
    Shader<void(Buffer<float4>, Buffer<float4>)> _gamma_correct;
    bool _gamma{true};

public:
    explicit RGBFilm(const FilmDesc &desc)
        : Film(desc),
          _rt_buffer(pipeline()->bindless_array()),
          _output_buffer(pipeline()->bindless_array()),
          _gamma(desc["gamma"].as_bool(true)) {}

    OC_SERIALIZABLE_FUNC(Film, _rt_buffer, _output_buffer)
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }

    void compile_accumulation() noexcept {
        Kernel kernel = [&](BufferVar<float4> input, BufferVar<float4> output, Uint frame_index) {
            Float4 accum_prev = output.read(dispatch_id());
            Float4 val = input.read(dispatch_id());
            Float a = 1.f / (frame_index + 1);
            val = lerp(make_float4(a), accum_prev, val);
            output.write(dispatch_id(), val);
        };
        _accumulate = device().compile(kernel, "RGBFilm-accumulation");
    }

    void compile_tone_mapping() noexcept {
        Kernel kernel = [&](BufferVar<float4> input, BufferVar<float4> output) {
            Float4 val = input.read(dispatch_id());
            val = _tone_mapper->apply(val);
            val.w = 1.f;
            output.write(dispatch_id(), val);
        };
        _tone_mapping = device().compile(kernel, "RGBFilm-tone_mapping");
    }

    void compile_gamma_correction() noexcept {
        Kernel kernel = [&](BufferVar<float4> input, BufferVar<float4> output) {
            Float4 val = input.read(dispatch_id());
            val = linear_to_srgb(val);
            val.w = 1.f;
            output.write(dispatch_id(), val);
        };
        _gamma_correct = device().compile(kernel, "RGBFilm-gamma_correction");
    }

    void compile() noexcept override {
        compile_accumulation();
        compile_tone_mapping();
        compile_gamma_correction();
    }
    void prepare() noexcept override {
        auto prepare = [&](RegistrableManaged<float4> &managed, const string &desc = "") noexcept {
            managed.set_bindless_array(pipeline()->bindless_array());
            managed.reset_all(device(), pixel_num(), desc);
            managed.reset_immediately();
            managed.register_self();
        };
        prepare(_rt_buffer, "RGBFilm::_rt_buffer");
        prepare(_accumulation_buffer, "RGBFilm::_accumulation_buffer");
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
    [[nodiscard]] CommandList accumulate(uint frame_index) const noexcept override {
        CommandList ret;
        ret << _accumulate(_rt_buffer.device_buffer(),
                           _accumulation_buffer.device_buffer(),
                           frame_index)
                   .dispatch(resolution());
        return ret;
    }
    [[nodiscard]] CommandList tone_mapping() const noexcept override {
        CommandList ret;
        ret << _tone_mapping(_accumulation_buffer.device_buffer(),
                             _output_buffer.device_buffer())
                   .dispatch(resolution());
        return ret;
    }
    [[nodiscard]] CommandList gamma_correct() const noexcept override {
        CommandList ret;
        ret << _gamma_correct(_accumulation_buffer.device_buffer(),
                              _output_buffer.device_buffer())
                   .dispatch(resolution());
        return ret;
    }
    [[nodiscard]] const RegistrableManaged<float4> &output_buffer() const noexcept override { return _output_buffer; }
    [[nodiscard]] RegistrableManaged<float4> &output_buffer() noexcept override { return _output_buffer; }
    [[nodiscard]] const RegistrableManaged<float4> &accumulation_buffer() const noexcept override { return _accumulation_buffer; }
    [[nodiscard]] RegistrableManaged<float4> &accumulation_buffer() noexcept override { return _accumulation_buffer; }
    [[nodiscard]] const RegistrableManaged<float4> &rt_buffer() const noexcept override { return _rt_buffer; }
    [[nodiscard]] RegistrableManaged<float4> &rt_buffer() noexcept override { return _rt_buffer; }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)