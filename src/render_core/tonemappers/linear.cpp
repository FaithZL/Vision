//
// Created by Zero on 2023/5/30.
//

#include "base/tonemapper.h"

namespace vision {

class LinearToneMapper : public ToneMapper {
public:
    explicit LinearToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc) {}
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return input;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::LinearToneMapper)