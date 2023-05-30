//
// Created by Zero on 2023/5/30.
//

#include "base/tonemapping.h"

namespace vision {

class ReinhardToneMapping : public ToneMapping {
public:
    explicit ReinhardToneMapping(const ToneMappingDesc &desc)
        : ToneMapping(desc) {}
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return input / (input + 1.f);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ReinhardToneMapping)