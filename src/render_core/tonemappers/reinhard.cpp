//
// Created by Zero on 2023/5/30.
//

#include "base/sensor/tonemapper.h"

namespace vision {

class ReinhardToneMapper : public ToneMapper {
public:
    explicit ReinhardToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return input / (input + 1.f);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ReinhardToneMapper)