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
    std::shared_future<Shader<void(uint)>> _combine;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : RayTracingIntegrator(desc),
          _direct(this, desc["direct"]),
          _indirect(this, desc["indirect"]) {
        _max_depth = _max_depth.hv() - 1;
    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void prepare() noexcept override {
        _direct.prepare();
        _indirect.prepare();
        Pipeline *rp = pipeline();
        auto init_buffer = [&]<typename T>(RegistrableBuffer<T> &buffer, const string &desc = "") {
            buffer.super() = device().create_buffer<T>(rp->pixel_num(), desc);
            vector<T> vec{rp->pixel_num(), T{}};
            buffer.upload_immediately(vec.data());
            buffer.register_self();
        };
        init_buffer(_motion_vectors, "RealTimeIntegrator::_motion_vectors");
        init_buffer(_hit_contexts, "RealTimeIntegrator::_ray_hits");
        init_buffer(_direct_light, "RealTimeIntegrator::_direct_light");
        init_buffer(_indirect_light, "RealTimeIntegrator::_indirect_light");

        _surfaces.super() = device().create_buffer<SurfaceData>(rp->pixel_num() * 2, "RealTimeIntegrator::_surfaces x 2");
        _surfaces.register_self(0, rp->pixel_num());
        _surfaces.register_view(rp->pixel_num(), rp->pixel_num());
    }

    void compile() noexcept override {
        _direct.compile();
        _indirect.compile();

        Camera *camera = scene().camera().get();
        Kernel kernel = [&](Uint frame_index) {
            camera->load_data();
            Float3 direct = direct_light().read(dispatch_id());
            Float3 indirect = indirect_light().read(dispatch_id());
            Float3 L = direct + indirect;
            camera->radiance_film()->add_sample(dispatch_idx().xy(), L, frame_index);
        };
        _combine = async([&, kernel = ocarina::move(kernel)] {
            return device().compile(kernel, "combine");
        });
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << Env::debugger().upload();
        stream << _direct.estimate(_frame_index);
        stream << _indirect.estimate(_frame_index);
        stream << _combine.get()(_frame_index).dispatch(pipeline()->resolution());
        stream << synchronize();
        stream << commit();
        Env::debugger().reset_range();
        _frame_index += 1;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeIntegrator)