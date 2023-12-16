//
// Created by Zero on 16/12/2023.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"
#include "base/color/spectrum.h"

namespace vision {

//    "type": "black",
//    "param": {
//
//    }
class Black : public Environment {
public:
    [[nodiscard]] bool is_black() const noexcept { return true; }
    explicit Black(const LightDesc &desc)
        : Environment(desc, LightType::Infinite) {}
    [[nodiscard]] SampledSpectrum Le(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        return SampledSpectrum{swl.dimension(), 0.f};
    }

    [[nodiscard]] Float PDF_wi(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        return 1.f;
    }
};

}// namespace vision