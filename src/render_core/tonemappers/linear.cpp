//
// Created by Zero on 2023/5/30.
//

#include "base/sensor/tonemapper.h"

namespace vision {

class LinearToneMapper : public ToneMapperImpl {
public:
    explicit LinearToneMapper(const ToneMapperDesc &desc)
        : ToneMapperImpl(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return input;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::LinearToneMapper)