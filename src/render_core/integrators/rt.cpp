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

class RealTimeIntegrator : public IlluminationIntegrator,
                           public enable_shared_from_this<RealTimeIntegrator>,
                           public Observer {
private:
    SP<ReSTIRDI> direct_;
    SP<ReSTIRGI> indirect_;
    SP<ScreenBuffer> specular_buffer_{make_shared<ScreenBuffer>("RealTimeIntegrator::specular_buffer_")};
    Shader<void(uint, float, float)> combine_;
    Shader<void(uint, Buffer<SurfaceData>)> path_tracing_;

public:
    RealTimeIntegrator() = default;
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc),
          direct_(make_shared<ReSTIRDI>(this, desc["direct"])),
          indirect_(make_shared<ReSTIRGI>(this, desc["indirect"])) {
        max_depth_ = max_depth_.hv() - 1;
    }

    VS_MAKE_GUI_STATUS_FUNC(IlluminationIntegrator, direct_, indirect_)

    void restore(vision::RuntimeObject *old_obj) noexcept override {
        IlluminationIntegrator::restore(old_obj);
        VS_HOTFIX_MOVE_ATTRS(direct_, indirect_, specular_buffer_,
                             combine_, path_tracing_, denoiser_)
        direct_->set_integrator(this);
        indirect_->set_integrator(this);
    }

    void update_resolution(ocarina::uint2 res) noexcept override {
        direct_->update_resolution(res);
        indirect_->update_resolution(res);
    }

    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override {
        std::tuple tp = {addressof(direct_), addressof(indirect_)};
        HotfixSystem::replace_objects(constructor, tp);
    }

    VS_MAKE_PLUGIN_NAME_FUNC
    void prepare() noexcept override {
        IlluminationIntegrator::prepare();
        direct_->prepare();
        indirect_->prepare();
        denoiser_->prepare();
        Pipeline *rp = pipeline();

        frame_buffer().prepare_screen_buffer(specular_buffer_);
        frame_buffer().prepare_hit_bsdfs();
        frame_buffer().prepare_surfaces();
        frame_buffer().prepare_albedo();
        frame_buffer().prepare_emission();
        frame_buffer().prepare_surface_exts();
        frame_buffer().prepare_hit_buffer();
        frame_buffer().prepare_gbuffer();
        frame_buffer().prepare_normal();
        frame_buffer().prepare_motion_vectors();
    }

    [[nodiscard]] RadianceCollector *rad_collector() noexcept { return scene().rad_collector(); }
    [[nodiscard]] const RadianceCollector *rad_collector() const noexcept { return scene().rad_collector(); }

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        direct_->render_UI(widgets);
        indirect_->render_UI(widgets);
    }

    void compile_path_tracing() noexcept {
        TSampler &sampler = scene().sampler();
        TSensor &camera = scene().sensor();
        Kernel kernel = [&](Uint frame_index, BufferVar<SurfaceData> surfaces) {
            SurfaceDataVar surface = surfaces.read(dispatch_id());
            $if(!surface->near_specular()) {
                specular_buffer_->write(dispatch_id(), make_float4(0.f));
                $return();
            };
            load_data();
            sampler->load_data();
            camera->load_data();
            Uint2 pixel = dispatch_idx().xy();
            sampler->start(pixel, frame_index, 5);
            RenderEnv render_env;
            render_env.initial(sampler, frame_index, spectrum());
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            Float scatter_pdf = 1e16f;
            RayState rs = camera->generate_ray(ss);
            Float3 L = Li(rs, scatter_pdf, spectrum()->one(), {}, render_env) * ss.filter_weight;
            specular_buffer_->write(dispatch_id(), make_float4(L, 1.f));
        };
        path_tracing_ = device().compile(kernel, "real_time_pt");
    }

    [[nodiscard]] CommandList compute_GBuffer() const noexcept {
        return frame_buffer().compute_GBuffer(frame_index_, frame_buffer().cur_gbuffer_view(frame_index_),
                                              frame_buffer().motion_vectors(), frame_buffer().albedo(),
                                              frame_buffer().emission(), frame_buffer().normal());
    }

    void compile() noexcept override {
        direct_->compile();
        indirect_->compile();
        denoiser_->compile();
        //        compile_path_tracing();

        TSensor &camera = scene().sensor();
        Kernel kernel = [&](Uint frame_index, Float di, Float ii) {
            camera->load_data();
            Float3 direct = direct_->radiance()->read(dispatch_id()).xyz() * di;
            Float3 indirect = indirect_->radiance()->read(dispatch_id()).xyz() * ii;
            Float3 L = direct + indirect;
            camera->rad_collector()->add_sample(dispatch_idx().xy(), L, frame_index);
        };
        combine_ = device().compile(kernel, "combine");
    }

    RealTimeDenoiseInput denoise_input() const noexcept {
        RealTimeDenoiseInput ret;
        TSensor &camera = scene().sensor();
        ret.frame_index = frame_index_;
        ret.resolution = pipeline()->resolution();
        ret.gbuffer = frame_buffer().cur_gbuffer_view(frame_index_);
        ret.prev_gbuffer = frame_buffer().prev_gbuffer_view(frame_index_);
        ret.motion_vec = frame_buffer().motion_vectors();
        ret.radiance = rad_collector()->rt_buffer();
        ret.albedo = frame_buffer().albedo();
        ret.emission = frame_buffer().emission();
        ret.output = rad_collector()->output_buffer();
        return ret;
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << frame_buffer().compute_hit(frame_index_);
        stream << compute_GBuffer();
        stream << direct_->dispatch(frame_index_);
        //        stream << path_tracing_(frame_index_,
        //                                frame_buffer().cur_surfaces(frame_index_))
        //                      .dispatch(pipeline()->resolution());
        stream << indirect_->dispatch(frame_index_);
        stream << combine_(frame_index_, direct_->factor(),
                           indirect_->factor())
                      .dispatch(pipeline()->resolution());
        auto dn_input = denoise_input();
        stream << denoiser_->dispatch(dn_input);
        increase_frame_index();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, RealTimeIntegrator)