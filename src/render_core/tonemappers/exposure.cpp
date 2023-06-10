//
// Created by Zero on 2023/5/30.
//

#include "base/tonemapping.h"

namespace vision {

class ExposureToneMapper : public ToneMapper {
private:
    float _exposure{};
public:
    explicit ExposureToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc),
          _exposure(desc["exposure"].as_float(1.f)) {}
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return 1.f - exp(-input * _exposure);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ExposureToneMapper)