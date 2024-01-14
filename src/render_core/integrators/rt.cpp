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

class RealTimeIntegrator : public RayTracingIntegrator {
private:
    ReSTIRDirectIllumination _direct;
    ReSTIRIndirectIllumination _indirect;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : RayTracingIntegrator(desc),
          _direct(this, desc["direct"], _motion_vectors, _surfaces, _rays),
          _indirect(this, desc["indirect"], _motion_vectors, _surfaces, _rays) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void prepare() noexcept override {
        _direct.prepare();
        _indirect.prepare();
        Pipeline *rp = pipeline();
        auto init_buffer = [&]<typename T>(RegistrableBuffer<T> &buffer, const string &desc = "") {
            buffer.super() = device().create_buffer<T>(rp->pixel_num(), desc);
            buffer.register_self();
        };
        init_buffer(_motion_vectors, "RealTimeIntegrator::_motion_vectors");
        init_buffer(_rays, "RealTimeIntegrator::_rays");
        init_buffer(_hits, "RealTimeIntegrator::_hits");

        _surfaces.super() = device().create_buffer<SurfaceData>(rp->pixel_num() * 2, "RealTimeIntegrator::_surfaces x 2");
        _surfaces.register_self(0, rp->pixel_num());
        _surfaces.register_view(rp->pixel_num(), rp->pixel_num());
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