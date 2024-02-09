//
// Created by Zero on 2024/2/8.
//

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/denoiser.h"

namespace vision {
using namespace ocarina;
class SVGF : public Denoiser {

public:
    explicit SVGF(const DenoiserDesc &desc)
        : Denoiser(desc) {
    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void apply(vision::DenoiseInput &input) noexcept override {

    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SVGF)