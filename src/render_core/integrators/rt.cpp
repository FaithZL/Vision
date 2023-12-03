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
    ReSTIRDirectIllumination _direct;
    ReSTIRIndirectIllumination _indirect;
    RegistrableManaged<float2> _motion_vectors;
    RegistrableManaged<SurfaceData> _surfaces0;
    RegistrableManaged<SurfaceData> _surfaces1;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc),
          _direct(desc["direct"], _motion_vectors, _surfaces0, _surfaces1),
          _indirect(desc["indirect"], _motion_vectors, _surfaces0, _surfaces1) {}

    void invalidation() const noexcept override {}

    void prepare() noexcept override {
        _direct.prepare();
        Pipeline *rp = pipeline();
        _motion_vectors.set_resource_array(rp->resource_array());
        _motion_vectors.reset_all(device(), rp->pixel_num());
        _motion_vectors.register_self();

        _surfaces0.set_resource_array(rp->resource_array());
        _surfaces0.reset_all(device(), rp->pixel_num());
        _surfaces0.register_self();

        _surfaces1.set_resource_array(rp->resource_array());
        _surfaces1.reset_all(device(), rp->pixel_num());
        _surfaces1.register_self();
    }

    void compile() noexcept override {
        _direct.compile();
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << Env::debugger().upload();
        stream << _direct.estimate(_frame_index++);
        stream << synchronize();
        stream << commit();
        Env::debugger().reset_range();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeIntegrator)