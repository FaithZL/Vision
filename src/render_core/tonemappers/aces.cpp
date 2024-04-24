//
// Created by Zero on 2023/5/30.
//

#include "base/sensor/tonemapper.h"

namespace vision {

class ACESToneMapper : public ToneMapperImpl {
public:
    explicit ACESToneMapper(const ToneMapperDesc &desc)
        : ToneMapperImpl(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] Float4 apply(const ocarina::Float4 &x) const noexcept override {
        float a = 2.51f;
        float b = 0.03f;
        float c = 2.43f;
        float d = 0.59f;
        float e = 0.14f;
        return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ACESToneMapper)