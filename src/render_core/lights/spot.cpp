//
// Created by Zero on 26/11/2022.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"

namespace vision {

//    "type" : "spot",
//    "param" : {
//        "angle" : 20,
//        "falloff" : 2,
//        "direction" : [0.2, -1, 0],
//        "color" : [1, 1, 1],
//        "position" : [-0.5, 1.9, 0],
//        "scale" : 1
//    }
class SpotLight : public IPointLight {
private:
    EncodedData<float3> position_;
    EncodedData<float3> direction_;
    EncodedData<float> angle_;
    // falloff angle range
    EncodedData<float> falloff_;

public:
    SpotLight() = default;
    explicit SpotLight(const LightDesc &desc)
        : IPointLight(desc),
          position_(desc["position"].as_float3()),
          angle_(radians(ocarina::clamp(desc["angle"].as_float(45.f), 1.f, 89.f))),
          falloff_(radians(ocarina::clamp(desc["falloff"].as_float(10.f), 0.f, angle_.hv()))),
          direction_(normalize(desc["direction"].as_float3(float3(0, 0, 1)))) {}
    OC_ENCODABLE_FUNC(IPointLight, position_, direction_, angle_, falloff_)
    VS_HOTFIX_MAKE_RESTORE(IPointLight, position_, direction_, angle_, falloff_)
    VS_MAKE_PLUGIN_NAME_FUNC
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        IPointLight::render_sub_UI(widgets);
        changed_ |= widgets->drag_float3("direction", &direction_.hv(), 0.02, 0, 0);
        changed_ |= widgets->slider_float("angle", &angle_.hv(), radians(1.f), radians(89.f));
        changed_ |= widgets->slider_float("fall off", &falloff_.hv(), 0.001, angle_.hv());
    }
    [[nodiscard]] float3 power() const noexcept override {
        return 2 * Pi * average() * (1 - .5f * (angle_.hv() * 2 + falloff_.hv()));
    }
    [[nodiscard]] Float3 position() const noexcept override { return *position_; }
    [[nodiscard]] float3 &host_position() noexcept override {
        return position_.hv();
    }
    [[nodiscard]] Float falloff(Float cos_theta) const noexcept {
        Float falloff_start = max(0.f, *angle_ - *falloff_);
        Float cos_angle = cos(*angle_);
        Float cos_falloff_start = cos(falloff_start);
        cos_theta = clamp(cos_theta, cos_angle, cos_falloff_start);
        Float factor = (cos_theta - cos_angle) / (cos_falloff_start - cos_angle);
        Float ret = Pow<4>(factor);
        return ret;
    }
    [[nodiscard]] Float3 direction(const LightSampleContext &p_ref) const noexcept override {
        return *direction_;
    }
    [[nodiscard]] SampledSpectrum Le(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        Float3 w_un = p_ref.pos - position();
        Float3 w = normalize(w_un);
        SampledSpectrum value = color_.eval_illumination_spectrum(p_light.uv, swl).sample * scale();
        return value / length_squared(w_un) * falloff(dot(*direction_, w));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, SpotLight)