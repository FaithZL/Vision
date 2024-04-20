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
    ReSTIRDI direct_;
    ReSTIRGI indirect_;
    Shader<void(uint, float, float)> combine_;
    SP<Denoiser> denoiser_;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc),
          direct_(this, desc["direct"]),
          indirect_(this, desc["indirect"]),
          denoiser_(NodeMgr::instance().load<Denoiser>(desc.denoiser_desc)) {
        max_depth_ = max_depth_.hv() - 1;
    }

    VS_MAKE_GUI_STATUS_FUNC(IlluminationIntegrator, direct_, indirect_)

    VS_MAKE_PLUGIN_NAME_FUNC
    void prepare() noexcept override {
        IlluminationIntegrator::prepare();
        direct_.prepare();
        indirect_.prepare();
        denoiser_->prepare();
        Pipeline *rp = pipeline();

        frame_buffer().prepare_bufferA();
        frame_buffer().prepare_bufferB();
        frame_buffer().prepare_hit_bsdfs();
        frame_buffer().prepare_surfaces();
        frame_buffer().prepare_hit_buffer();
        frame_buffer().prepare_motion_vectors();
    }

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        direct_.render_UI(widgets);
        indirect_.render_UI(widgets);
    }

    void compile() noexcept override {
        frame_buffer().compile();
        direct_.compile();
        indirect_.compile();

        Camera *camera = scene().camera().get();
        Kernel kernel = [&](Uint frame_index, Float di, Float ii) {
            camera->load_data();
            Float3 direct = frame_buffer().bufferA().read(dispatch_id()).xyz() * di;
            Float3 indirect = frame_buffer().bufferB().read(dispatch_id()).xyz() * ii;
            Float3 L = direct + indirect;
            camera->film()->add_sample(dispatch_idx().xy(), L, frame_index);
        };
        combine_ = device().compile(kernel, "combine");
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << Env::debugger().upload();
        stream << frame_buffer().compute_hit();
        stream << direct_.estimate(frame_index_);
        stream << indirect_.estimate(frame_index_);
        stream << combine_(frame_index_, direct_.factor(),
                           indirect_.factor())
                      .dispatch(pipeline()->resolution());
        stream << synchronize();
        stream << commit();
        Env::debugger().reset_range();
        increase_frame_index();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeIntegrator)