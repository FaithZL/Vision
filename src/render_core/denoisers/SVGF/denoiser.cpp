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
    void prepare() noexcept override {
        _reproject.prepare();
        _filter_moment.prepare();
        _atrous.prepare();
    }

    void compile() noexcept {
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