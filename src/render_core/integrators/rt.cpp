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
    RegistrableManaged<SurfaceData> _surfaces;
    RegistrableManaged<SurfaceData> _prev_surfaces;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc),
          _direct(desc["direct"], _motion_vectors, _surfaces, _prev_surfaces) {}

    void invalidation() const noexcept override {}

    void prepare() noexcept override {
        _direct.prepare();
        Pipeline *rp = pipeline();
        _motion_vectors.set_resource_array(rp->resource_array());
        _motion_vectors.reset_all(device(), rp->pixel_num());
        _motion_vectors.register_self();

        _surfaces.set_resource_array(rp->resource_array());
        _surfaces.reset_all(device(), rp->pixel_num());
        _surfaces.register_self();

        _prev_surfaces.set_resource_array(rp->resource_array());
        _prev_surfaces.reset_all(device(), rp->pixel_num());
        _prev_surfaces.register_self();
    }

    void compile() noexcept override {
        _direct.compile();
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << _direct.estimate(_frame_index++);
        stream << synchronize();
        stream << commit();
        float2 vec = _motion_vectors.at(0);
        //        if (vec.x > 0.01)
        //            cout << vec.x << "  " << vec.y << endl;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeIntegrator)