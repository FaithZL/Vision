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
    SP<Denoiser> _denoiser;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : RayTracingIntegrator(desc),
          _direct(this, desc["direct"]),
          _indirect(this, desc["indirect"]),
          _denoiser(NodeMgr::instance().load<Denoiser>(desc.denoiser_desc)) {
        _max_depth = _max_depth.hv() - 1;
    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void prepare() noexcept override {
        _direct.prepare();
        _indirect.prepare();
        _denoiser->prepare();
        pipeline()->frame_buffer()->prepare();
        Pipeline *rp = pipeline();
        auto init_buffer = [&]<typename T>(RegistrableBuffer<T> &buffer, const string &desc = "") {
            buffer.super() = device().create_buffer<T>(rp->pixel_num(), desc);
            vector<T> vec{rp->pixel_num(), T{}};
            buffer.upload_immediately(vec.data());
            buffer.register_self();
        };
        init_buffer(frame_buffer().hit_bsdfs(), "RealTimeIntegrator::_hit_bsdfs");
        init_buffer(_radiance0, "RealTimeIntegrator::_radiance0");
        init_buffer(_radiance1, "RealTimeIntegrator::_radiance1");
    }

    void compile() noexcept override {
        _direct.compile();
        _indirect.compile();

        Camera *camera = scene().camera().get();
        Kernel kernel = [&](Uint frame_index) {
            camera->load_data();
            Float3 direct = radiance0().read(dispatch_id()) * _direct.factor();
            Float3 indirect = radiance1().read(dispatch_id()) * _indirect.factor();
            Float3 L = direct + indirect;
            camera->radiance_film()->add_sample(dispatch_idx().xy(), L, frame_index);
        };
        _combine = device().async_compile(ocarina::move(kernel), "combine");
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
        increase_frame_index();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeIntegrator)