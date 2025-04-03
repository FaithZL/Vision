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
    EncodedData<vector<float>> lenses_;
};

}// namespace vision