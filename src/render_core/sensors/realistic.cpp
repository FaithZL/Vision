//
// Created by Zero on 2025/4/3.
//

#include "base/sensor/sensor.h"
#include "base/mgr/pipeline.h"
#include "GUI/widgets.h"

namespace vision {
struct LensInterface {
    float curvature_radius{};
    float thickness{};
    float eta{};
    float aperture_radius{};
};
}// namespace vision

static constexpr std::array lens_config = {
    float4(35.98738, 1.21638, 1.54, 23.716),
    float4(11.69718, 9.9957, 1, 17.996),
    float4(13.08714, 5.12622, 1.772, 12.364),
    float4(-22.63294, 1.76924, 1.617, 9.812),
    float4(71.05802, 0.8184, 1, 9.152),
    float4(0, 2.27766, 0, 8.756),
    float4(-9.58584, 2.43254, 1.617, 8.184),
    float4(-11.28864, 0.11506, 1, 9.152),
    float4(-166.7765, 3.09606, 1.713, 10.648),
    float4(-7.5911, 1.32682, 1.805, 11.44),
    float4(-16.7662, 3.98068, 1, 12.276),
    float4(-7.70286, 1.21638, 1.617, 13.42),
    float4(-11.97328, 1.66, 1, 17.996),
};

// clang-format off
OC_STRUCT(vision, LensInterface, curvature_radius,
          thickness, eta, aperture_radius){
    void init(const Float &c, const Float &t, const Float &e, const Float &a) noexcept {
        curvature_radius = c;
        thickness = t;
        eta = e;
        aperture_radius = a;
    }
};
// clang-format on

namespace vision {

class RealisticCamera : public Camera {
private:
    EncodedData<vector<float>> lens_interfaces_;
    bool simple_weighting_{};

public:
    RealisticCamera() = default;
    explicit RealisticCamera(const SensorDesc &desc)
        : Camera(desc) {}
    OC_ENCODABLE_FUNC(Camera, lens_interfaces_)
    [[nodiscard]] LensInterfaceVar lens_element(const Uint &index) const noexcept {
        Uint i = index * 4;
        Float curvature_radius = (*lens_interfaces_)[i];
        Float thickness = (*lens_interfaces_)[i + 1];
        Float eta = (*lens_interfaces_)[i + 2];
        Float aperture_radius = (*lens_interfaces_)[i + 3];
        LensInterfaceVar ret;
        ret->init(curvature_radius, thickness, eta, aperture_radius);
        return ret;
    }
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, RealisticCamera)
VS_REGISTER_CURRENT_PATH(0, "vision-sensor-realistic.dll")