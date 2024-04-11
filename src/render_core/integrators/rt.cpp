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
    std::shared_future<Shader<void(uint)>> _combine;
    SP<Denoiser> _denoiser;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc),
          _direct(this, desc["direct"]),
          _indirect(this, desc["indirect"]),
          _denoiser(NodeMgr::instance().load<Denoiser>(desc.denoiser_desc)) {
        _max_depth = _max_depth.hv() - 1;
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void prepare() noexcept override {
        IlluminationIntegrator::prepare();
        _direct.prepare();
        _indirect.prepare();
        _denoiser->prepare();
        Pipeline *rp = pipeline();

        frame_buffer().prepare_bufferA();
        frame_buffer().prepare_bufferB();
        frame_buffer().prepare_hit_bsdfs();
        frame_buffer().prepare_surfaces();
        frame_buffer().prepare_motion_vectors();
    }

    void compile() noexcept override {
        _direct.compile();
        _indirect.compile();

        Camera *camera = scene().camera().get();
        Kernel kernel = [&](Uint frame_index) {
            camera->load_data();
            Float3 direct = frame_buffer().bufferA().read(dispatch_id()).xyz() * _direct.factor();
            Float3 indirect = frame_buffer().bufferB().read(dispatch_id()).xyz() * _indirect.factor();
            Float3 L = direct + indirect;
            camera->film()->add_sample(dispatch_idx().xy(), L, frame_index);
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