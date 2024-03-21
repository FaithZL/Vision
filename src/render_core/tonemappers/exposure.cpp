//
// Created by Zero on 2023/5/30.
//

#include "base/sensor/tonemapper.h"

namespace vision {

class ExposureToneMapper : public ToneMapper {
private:
    Serial<float> _exposure{};

public:
    explicit ExposureToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc),
          _exposure(desc["exposure"].as_float(1.f)) {}
    OC_SERIALIZABLE_FUNC(ToneMapper, _exposure)
    VS_MAKE_PLUGIN_NAME_FUNC

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        _changed |= widgets->input_float("exposure", addressof(_exposure.hv()), 0.1, 0.5);
    }
    
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        _exposure.decode();
        return 1.f - exp(-input * *_exposure);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ExposureToneMapper)