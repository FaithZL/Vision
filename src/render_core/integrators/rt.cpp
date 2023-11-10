//
// Created by Zero on 2023/9/11.
//

#include "base/integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"
#include "ReSTIR/direct.h"
#include "ReSTIR/indirect.h"

namespace vision {

class RealTimeIntegrator : public IlluminationIntegrator {
private:
    ReSTIR _direct;
    ReSTIRIndirectIllumination _indirect;
    RegistrableManaged<float2> _motion_vectors;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc),
          _direct(desc["direct"], _motion_vectors) {}

    void prepare() noexcept override {
        _direct.prepare();
        Pipeline *rp = pipeline();
        _motion_vectors.set_resource_array(rp->resource_array());
        _motion_vectors.reset_all(device(), rp->pixel_num());
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
        float2 vec = _motion_vectors.at(0);
//        if (vec.x > 0.01)
//            cout << vec.x << "  " << vec.y << endl;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeIntegrator)