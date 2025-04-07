//
// Created by Zero on 2025/4/3.
//

#include "base/sensor/sensor.h"
#include "base/mgr/pipeline.h"
#include "GUI/widgets.h"

namespace vision {
struct LensElement {
    float curvature_radius{};
    float thickness{};
    float eta{};
    float aperture_radius{};
};
}// namespace vision

// clang-format off
OC_STRUCT(vision, LensElement, curvature_radius,
          thickness, ior, aperture_radius){
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
    ///
    EncodedData<vector<float>> lenses_;
    bool simple_weighting_{};

public:
    RealisticCamera() = default;
    explicit RealisticCamera(const SensorDesc &desc)
        : Camera(desc) {}
    OC_ENCODABLE_FUNC(Camera, lenses_)
    [[nodiscard]] LensElementVar lens(const Uint &index) const noexcept {
        Uint i = index * 4;
        Float curvature_radius = (*lenses_)[i];
        Float thickness = (*lenses_)[i + 1];
        Float eta = (*lenses_)[i + 2];
        Float aperture_radius = (*lenses_)[i + 3];
        LensElementVar ret;
        ret->init(curvature_radius, thickness, eta, aperture_radius);
        return ret;
    }
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, RealisticCamera)
VS_REGISTER_CURRENT_PATH(0, "vision-sensor-realistic.dll")