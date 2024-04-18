//
// Created by Zero on 2023/5/30.
//

#include "base/sensor/tonemapper.h"

namespace vision {

class ExposureToneMapper : public ToneMapper {
private:
    Serial<float> exposure_{};

public:
    explicit ExposureToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc),
          exposure_(desc["exposure"].as_float(1.f)) {}
    OC_SERIALIZABLE_FUNC(ToneMapper, exposure_)
    VS_MAKE_PLUGIN_NAME_FUNC

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        changed_ |= widgets->input_float("exposure", addressof(exposure_.hv()), 0.1, 0.5);
    }
    
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        exposure_.decode();
        return 1.f - exp(-input * *exposure_);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ExposureToneMapper)