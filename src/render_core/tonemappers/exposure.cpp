//
// Created by Zero on 2023/5/30.
//

#include "base/sensor/tonemapper.h"
#include "hotfix/hotfix.h"

namespace vision {

class ExposureToneMapper : public ToneMapper {
private:
    EncodedData<float> exposure_{};

public:
    ExposureToneMapper() = default;
    explicit ExposureToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc),
          exposure_(desc["exposure"].as_float(1.f)) {}
    OC_ENCODABLE_FUNC(ToneMapper, exposure_)
    VS_HOTFIX_MAKE_RESTORE(ToneMapper, exposure_)
    VS_MAKE_PLUGIN_NAME_FUNC

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        changed_ |= widgets->drag_float("exposure", addressof(exposure_.hv()), 0.01, 0, 5);
    }

    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return 1.f - exp(-input * *exposure_);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, ExposureToneMapper)
VS_REGISTER_CURRENT_PATH(0, "vision-tonemapper-exposure.dll")