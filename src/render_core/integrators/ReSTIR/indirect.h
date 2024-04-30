//
// Created by Zero on 2023/9/11.
//

#pragma once

#include "common.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"
#include "indirect_util.h"
#include "core/thread_pool.h"

namespace vision::indirect {
struct Param {
    uint max_age{};

    /// spatial
    uint spatial{1};
    uint N{};
    float s_dot{};
    float s_depth{};
    float s_radius{};

    /// temporal
    uint temporal{1};
    uint history_limit{};
    float t_dot{};
    float t_depth{};
    float t_radius{};
};
}// namespace vision::indirect

OC_PARAM_STRUCT(vision::indirect::Param, max_age, spatial, N,
                s_dot, s_depth, s_radius, temporal, history_limit,
                t_dot, t_depth, t_radius){};

namespace vision {

class RayTracingIntegrator;
using namespace vision::indirect;

class ReSTIRGI : public SerialObject, public Context, public RenderEnv, public GUI {
private:
    SpatialResamplingParam spatial_;
    TemporalResamplingParam temporal_;
    bool open_{true};
    uint max_age_{};
    IlluminationIntegrator *integrator_{};

    RegistrableBuffer<indirect::Reservoir> reservoirs_{pipeline()->bindless_array()};
    RegistrableBuffer<indirect::RSVSample> samples_{pipeline()->bindless_array()};

    /**
     * initial sample
     */
    Shader<void(uint)> initial_samples_;
    /**
     * initial samples and temporal reuse
     */
    Shader<void(indirect::Param, uint)> temporal_pass_;
    /**
     * spatial reuse and shading
     */
    Shader<void(indirect::Param, uint)> spatial_shading_;

protected:
    [[nodiscard]] static Sampler &sampler() noexcept { return scene().sampler(); }

public:
    ReSTIRGI(IlluminationIntegrator *integrator, const ParameterSet &desc);
    OC_MAKE_MEMBER_GETTER(open, )
    [[nodiscard]] float factor() const noexcept { return static_cast<float>(open()); }
    void prepare() noexcept;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    void compile_initial_samples() noexcept;
    void compile_temporal_reuse() noexcept;
    [[nodiscard]] ScatterEval eval_bsdf(const Interaction &it, const GIRSVSample &sample, MaterialEvalMode mode) const noexcept;
    [[nodiscard]] Float compute_p_hat(const Interaction &it, const GIRSVSample &sample) const noexcept;
    void compile_spatial_shading() noexcept;
    void compile() noexcept {
        compile_initial_samples();
        compile_temporal_reuse();
        compile_spatial_shading();
    }
    [[nodiscard]] Float Jacobian_det(Float3 cur_pos, Float3 neighbor_pos, Var<SurfacePoint> sample_point) const noexcept;
    [[nodiscard]] GIRSVSample init_sample(const Interaction &it, const SensorSample &ss,
                                          HitBSDFVar &hit_bsdf) noexcept;
    [[nodiscard]] GIReservoir combine_temporal(const GIReservoir &cur_rsv, SurfaceDataVar cur_surf,
                                               GIReservoir &other_rsv, SurfaceDataVar *neighbor_surf = nullptr) const noexcept;
    [[nodiscard]] GIReservoir temporal_reuse(GIReservoir rsv, const SurfaceDataVar &cur_surf,
                                             const Float2 &motion_vec, const SensorSample &ss,
                                             const Var<indirect::Param> &param) const noexcept;

    [[nodiscard]] GIReservoir constant_combine(const GIReservoir &canonical_rsv,
                                               const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] GIReservoir combine_spatial(GIReservoir cur_rsv, const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] GIReservoir spatial_reuse(GIReservoir rsv, const SurfaceDataVar &cur_surf,
                                            const Int2 &pixel, const Var<indirect::Param> &param) const noexcept;
    [[nodiscard]] Float3 shading(GIReservoir rsv, const SurfaceDataVar &cur_surf) const noexcept;

    [[nodiscard]] static Bool is_neighbor(const SurfaceDataVar &cur_surface,
                                          const SurfaceDataVar &another_surface,
                                          const Var<indirect::Param> &param) noexcept {
        return vision::is_neighbor(cur_surface, another_surface,
                                   param.s_dot,
                                   param.s_depth);
    }
    [[nodiscard]] static Bool is_temporal_valid(const SurfaceDataVar &cur_surface,
                                                const SurfaceDataVar &prev_surface,
                                                const Var<indirect::Param> &param,
                                                GIRSVSample *sample) noexcept {
        Bool cond = sample ? sample->age < param.max_age : true;
        return vision::is_neighbor(cur_surface, prev_surface,
                                   param.t_dot,
                                   param.t_depth) && cond;
    }
    [[nodiscard]] uint reservoir_base() const noexcept { return reservoirs_.index().hv(); }
    [[nodiscard]] auto prev_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().prev_surfaces_index(frame_index()));
    }
    [[nodiscard]] auto cur_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().cur_surfaces_index(frame_index()));
    }
    [[nodiscard]] auto prev_reservoirs() const noexcept {
        return pipeline()->buffer_var<indirect::Reservoir>((frame_index() & 1) + reservoir_base());
    }
    [[nodiscard]] auto cur_reservoirs() const noexcept {
        return pipeline()->buffer_var<indirect::Reservoir>(((frame_index() + 1) & 1) + reservoir_base());
    }
    [[nodiscard]] auto passthrough_reservoirs() const noexcept {
        return pipeline()->buffer_var<indirect::Reservoir>(2 + reservoir_base());
    }
    [[nodiscard]] indirect::Param construct_param() const noexcept;
    [[nodiscard]] CommandList estimate(uint frame_index) const noexcept;
};

}// namespace vision