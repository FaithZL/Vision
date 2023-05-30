//
// Created by Zero on 2023/5/30.
//

#include "base/tonemapping.h"

namespace vision {

class GammaToneMapping : public ToneMapping {
public:
    explicit GammaToneMapping(const ToneMappingDesc &desc)
        : ToneMapping(desc) {}
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return ocarina::linear_to_srgb(input);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GammaToneMapping)