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
    float ior{};
    float aperture_radius{};
};
}// namespace vision

OC_STRUCT(vision, LensElement, curvature_radius,
          thickness, ior, aperture_radius){

};

namespace vision {

class RealisticCamera : public Camera {
private:
    ///
    EncodedData<vector<float>> lenses_;
    bool simple_weighting_{};

public:
    RealisticCamera() = default;
    RealisticCamera(const SensorDesc &desc)
        : Camera(desc) {}
//    [[nodiscard]] LensElementVar lens(const Uint &index) const noexcept {
//
//    }
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, RealisticCamera)
VS_REGISTER_CURRENT_PATH(0, "vision-sensor-realistic.dll")