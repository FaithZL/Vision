//
// Created by Zero on 2023/6/14.
//

#include "base/integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"

namespace vision {

class AmbientOcclusionIntegrator : public Integrator {
private:
    float _distance{1.f};

public:
    explicit AmbientOcclusionIntegrator(const IntegratorDesc &desc)
        : Integrator(desc), _distance(desc["distance"].as_float(1.f)) {}

    void compile_shader() noexcept override {

    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << _shader(rp->frame_index()).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AmbientOcclusionIntegrator)