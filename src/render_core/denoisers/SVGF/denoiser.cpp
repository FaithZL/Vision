//
// Created by Zero on 2024/2/8.
//

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/denoiser.h"
#include "atrous.h"
#include "temporal.h"

namespace vision {
using namespace ocarina;
class SVGF : public Denoiser {
private:
    AtrousFilter _atrous;
    uint N;

public:
    explicit SVGF(const DenoiserDesc &desc)
        : Denoiser(desc),
          _atrous(desc.filter_desc),N(desc["N"].as_uint(3)) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void prepare() noexcept override {
        _atrous.prepare();
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