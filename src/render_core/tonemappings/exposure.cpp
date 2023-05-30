//
// Created by Zero on 2023/5/30.
//

#include "base/tonemapping.h"

namespace vision {

class ExposureToneMapping : public ToneMapping {
private:
    float _exposure{};
public:
    explicit ExposureToneMapping(const ToneMappingDesc &desc)
        : ToneMapping(desc),
          _exposure(desc["exposure"].as_float(1.f)) {}
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return 1.f - exp(-input * _exposure);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ExposureToneMapping)