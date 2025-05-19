//
// Created by Zero on 22/10/2022.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"
#include "base/color/spectrum.h"

namespace vision {

//"type" : "point",
//"param" : {
//    "color" : {
//        "channels" : "xyz",
//        "node" : [1, 1, 1]
//    },
//    "position" : [-0.5, 1.8, 0],
//    "scale" : 0.2
//}
class PointLight : public IPointLight {
private:
    EncodedData<float3> position_;

public:
    PointLight() = default;
    explicit PointLight(const LightDesc &desc)
        : IPointLight(desc),
          position_(desc["position"].as_float3()) {}
    OC_ENCODABLE_FUNC(IPointLight, position_)
    VS_HOTFIX_MAKE_RESTORE(IPointLight, position_)
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] float3 power() const noexcept override {
        return 4 * Pi * average();
    }
    [[nodiscard]] Float3 position() const noexcept override { return *position_; }
    [[nodiscard]] float3 &host_position() noexcept override {
        return position_.hv();
    }
    [[nodiscard]] SampledSpectrum Le(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum value = color_.eval_illumination_spectrum(p_light.uv, swl).sample * scale();
        return value / length_squared(p_ref.pos - position());
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, PointLight)