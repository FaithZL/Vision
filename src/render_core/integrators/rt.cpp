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
    RegistrableBuffer<float2> _motion_vectors;
    RegistrableBuffer<SurfaceData> _surfaces0;
    RegistrableBuffer<SurfaceData> _surfaces1;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc),
          _direct(this, desc["direct"], _motion_vectors, _surfaces0, _surfaces1),
          _indirect(desc["indirect"], _motion_vectors, _surfaces0, _surfaces1) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void prepare() noexcept override {
        _direct.prepare();
        Pipeline *rp = pipeline();
        auto init_buffer = [&]<typename T>(RegistrableBuffer<T> &buffer, const string &desc = "") {
            buffer.set_bindless_array(rp->bindless_array());
            buffer.super() = device().create_buffer<T>(rp->pixel_num(), desc);
            buffer.register_self();
        };
        init_buffer(_motion_vectors, "RealTimeIntegrator::_motion_vectors");
        init_buffer(_surfaces0, "RealTimeIntegrator::_surfaces0");
        init_buffer(_surfaces1, "RealTimeIntegrator::_surfaces1");
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