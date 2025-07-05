//
// Created by Zero on 2023/9/11.
//

#pragma once

#include "common.h"
#include "reservoir.h"

namespace vision {
struct GIParam {
    uint max_age{};
    float diff_factor{};

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
}// namespace vision

OC_PARAM_STRUCT(vision, GIParam, max_age, diff_factor, spatial, N,
                s_dot, s_depth, s_radius, temporal, history_limit,
                t_dot, t_depth, t_radius){};

namespace vision {

class RayTracingIntegrator;

class ReSTIRGI : public ReSTIR {
private:
    SP<ScreenBuffer> radiance_{make_shared<ScreenBuffer>("ReSTIRGI::radiance_")};
    RegistrableBuffer<GIReservoir> reservoirs_{pipeline()->bindless_array()};
    RegistrableBuffer<GISample> samples_{pipeline()->bindless_array()};

    /**
     * initial sample
     */
    Shader<void(uint)> initial_samples_;
    /**
     * initial samples and temporal reuse
     */
    Shader<void(GIParam, uint)> temporal_pass_;
    /**
     * spatial reuse and shading
     */
    Shader<void(GIParam, uint)> spatial_shading_;

protected:
    [[nodiscard]] static TSampler &sampler() noexcept { return scene().sampler(); }

public:
    ReSTIRGI() = default;
    ReSTIRGI(IlluminationIntegrator *integrator, const ParameterSet &desc);
    VS_HOTFIX_MAKE_RESTORE(ReSTIR, radiance_, reservoirs_, samples_,
                           initial_samples_, temporal_pass_, spatial_shading_)
    OC_MAKE_MEMBER_GETTER(open, )
    OC_MAKE_MEMBER_GETTER(radiance, &)
    OC_MAKE_MEMBER_SETTER(integrator)
    [[nodiscard]] float factor() const noexcept { return static_cast<float>(open()); }
    void prepare() noexcept;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    HOTFIX_VIRTUAL void compile_initial_samples() noexcept;
    HOTFIX_VIRTUAL void compile_temporal_reuse() noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL ScatterEval eval_bsdf(const Interaction &it, const GISampleVar &sample, MaterialEvalMode mode) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL Float compute_p_hat(const Interaction &it, const GISampleVar &sample) const noexcept;
    HOTFIX_VIRTUAL void compile_spatial_shading() noexcept;
    void compile() noexcept {
        compile_initial_samples();
        compile_temporal_reuse();
        compile_spatial_shading();
    }
    void update_resolution(ocarina::uint2 res) noexcept override;
    [[nodiscard]] HOTFIX_VIRTUAL Float Jacobian_det(Float3 cur_pos, Float3 neighbor_pos, Var<SurfacePoint> sample_point) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL GISampleVar init_sample(const Interaction &it, const SensorSample &ss,
                                                         HitBSDFVar &hit_bsdf) noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL GIReservoirVar combine_temporal(const GIReservoirVar &cur_rsv, SurfaceDataVar cur_surf,
                                                                 GIReservoirVar &other_rsv, SurfaceDataVar *neighbor_surf,
                                                                 Float3 view_pos, Float3 prev_view_pos) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL GIReservoirVar temporal_reuse(GIReservoirVar rsv, const SurfaceDataVar &cur_surf,
                                                               const Float2 &motion_vec, const SensorSample &ss,
                                                               const Var<GIParam> &param) const noexcept;

    [[nodiscard]] HOTFIX_VIRTUAL GIReservoirVar constant_combine(const GIReservoirVar &canonical_rsv,
                                                                 const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL GIReservoirVar combine_spatial(GIReservoirVar cur_rsv, const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL GIReservoirVar spatial_reuse(GIReservoirVar rsv, const SurfaceDataVar &cur_surf,
                                                              const Int2 &pixel, const Var<GIParam> &param) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL Float3 shading(GIReservoirVar rsv, const SurfaceDataVar &cur_surf) const noexcept;

    [[nodiscard]] static Bool is_valid_neighbor(const SurfaceDataVar &cur_surface,
                                                const SurfaceDataVar &another_surface,
                                                const Var<GIParam> &param) noexcept {
        return vision::is_valid_neighbor(cur_surface, another_surface,
                                         param.s_dot, param.s_depth,
                                         param.diff_factor);
    }
    [[nodiscard]] static Bool is_temporal_valid(const SurfaceDataVar &cur_surface,
                                                const SurfaceDataVar &prev_surface,
                                                const Var<GIParam> &param,
                                                GISampleVar *sample) noexcept {
        Bool cond = sample ? sample->age < param.max_age : true;
        return cond && vision::is_valid_neighbor(cur_surface, prev_surface,
                                                 param.t_dot, param.t_depth,
                                                 param.diff_factor);
    }
    [[nodiscard]] uint reservoir_base() const noexcept { return reservoirs_.index().hv(); }
    [[nodiscard]] auto prev_reservoirs() const noexcept {
        return pipeline()->buffer_var<GIReservoir>((frame_index() & 1) + reservoir_base());
    }
    [[nodiscard]] auto cur_reservoirs() const noexcept {
        return pipeline()->buffer_var<GIReservoir>(((frame_index() + 1) & 1) + reservoir_base());
    }
    [[nodiscard]] auto passthrough_reservoirs() const noexcept {
        return pipeline()->buffer_var<GIReservoir>(2 + reservoir_base());
    }
    [[nodiscard]] GIParam construct_param() const noexcept;
    [[nodiscard]] CommandList dispatch(uint frame_index) const noexcept;
};
}// namespace vision
