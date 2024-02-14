//
// Created by Zero on 2024/2/8.
//

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/denoiser.h"
#include "reproject.h"
#include "filter_moment.h"
#include "atrous.h"

namespace vision {
using namespace ocarina;
class SVGF : public Denoiser {
private:
    Buffer<float4> _prev_depth_normal;
    Buffer<float4> _prev_filtered_lighting;
    Buffer<float2> _prev_moment;

private:
    Reproject _reproject{this};
    FilterMoment _filter_moment{this};
    AtrousFilter _atrous;
    uint N;

public:
    explicit SVGF(const DenoiserDesc &desc)
        : Denoiser(desc),
          _atrous(desc.filter_desc, this),
          N(desc["N"].as_uint(3)) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }

    void prepare_buffers() {
        Pipeline *rp = pipeline();
        auto init_buffer = [&]<typename T>(Buffer<T> &buffer, const string &desc = "") {
            buffer = device().create_buffer<T>(rp->pixel_num(), desc);
            vector<T> vec{rp->pixel_num(), T{}};
            buffer.upload_immediately(vec.data());
        };
        init_buffer(_prev_depth_normal, "SVGF::_prev_depth_normal");
        init_buffer(_prev_filtered_lighting, "SVGF::_prev_filtered_lighting");
        init_buffer(_prev_moment, "SVGF::_prev_moment");
    }

    void prepare() noexcept override {
        prepare_buffers();
        _reproject.prepare();
        _filter_moment.prepare();
        _atrous.prepare();
    }

    void compile() noexcept override {
        _reproject.compile();
        _filter_moment.compile();
        _atrous.compile();
    }

    [[nodiscard]] CommandList dispatch(vision::DenoiseInput &input) noexcept override {
        CommandList ret;
        for (int i = 0; i < N; ++i) {
            uint step_width = pow(2, i);
            ret = _atrous.dispatch(input, step_width);
        }
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SVGF)