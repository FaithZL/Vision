//
// Created by Zero on 2023/9/11.
//

#include "base/integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"
#include "ReSTIR/direct.h"

namespace vision {

class RealTimeIntegrator : public IlluminationIntegrator {
private:
    ReSTIR _direct;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc),
          _direct(desc["M"].as_uint(1),
                  desc["n"].as_uint(3),
                  desc["spatial"].as_uint(1),
                  desc["theta"].as_float(20),
                  desc["depth"].as_float(0.01f)) {}

    void prepare() noexcept override {
        _direct.prepare();
    }

    void compile() noexcept override {
        _direct.compile();
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << _direct.estimate();
        stream << synchronize();
        stream << commit();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeIntegrator)