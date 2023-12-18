//
// Created by Zero on 2023/5/30.
//

#include "base/sensor/tonemapper.h"

namespace vision {

class LinearToneMapper : public ToneMapper {
public:
    explicit LinearToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return input;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::LinearToneMapper)