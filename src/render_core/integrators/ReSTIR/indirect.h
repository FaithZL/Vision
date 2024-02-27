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

namespace vision {

class RayTracingIntegrator;
using namespace vision::ReSTIRIndirect;

class ReSTIRIndirectIllumination : public SerialObject, public Context, public RenderEnv {
private:
    SpatialResamplingParam _spatial;
    TemporalResamplingParam _temporal;
    bool _open{true};
    IlluminationIntegrator *_integrator{};

    RegistrableBuffer<ReSTIRIndirect::Reservoir> _reservoirs{pipeline()->bindless_array()};
    RegistrableBuffer<ReSTIRIndirect::RSVSample> _samples{pipeline()->bindless_array()};

    /**
     * initial sample
     */
    std::shared_future<Shader<void(uint)>> _initial_samples;
    /**
     * initial samples and temporal reuse
     */
    std::shared_future<Shader<void(uint)>> _temporal_pass;
    /**
     * spatial reuse and shading
     */
    std::shared_future<Shader<void(uint)>> _spatial_shading;

protected:
    [[nodiscard]] static Sampler *sampler() noexcept { return scene().sampler(); }

public:
    ReSTIRIndirectIllumination(IlluminationIntegrator *integrator, const ParameterSet &desc);
    OC_MAKE_MEMBER_GETTER(open, )
    [[nodiscard]] float factor() const noexcept { return static_cast<float>(open()); }
    void prepare() noexcept;
    void compile_initial_samples() noexcept;
    void compile_temporal_reuse() noexcept;
    [[nodiscard]] ScatterEval eval_bsdf(const Interaction &it, const IIRSVSample &sample, MaterialEvalMode mode) const noexcept;
    [[nodiscard]] Float compute_p_hat(const Interaction &it, const IIRSVSample &sample) const noexcept;
    void compile_spatial_shading() noexcept;
    void compile() noexcept {
        compile_initial_samples();
        compile_temporal_reuse();
        compile_spatial_shading();
    }
    [[nodiscard]] Float Jacobian_det(Float3 cur_pos, Float3 neighbor_pos, Var<SurfacePoint> sample_point) const noexcept;
    [[nodiscard]] IIRSVSample init_sample(const Interaction &it, const SensorSample &ss,
                                          OCHitBSDF &hit_bsdf) noexcept;
    [[nodiscard]] IIReservoir combine_temporal(const IIReservoir &cur_rsv, OCSurfaceData cur_surf,
                                               const IIReservoir &other_rsv) const noexcept;
    [[nodiscard]] IIReservoir temporal_reuse(IIReservoir rsv, const OCSurfaceData &cur_surf,
                                             const Float2 &motion_vec, const SensorSample &ss) const noexcept;

    [[nodiscard]] IIReservoir constant_combine(const IIReservoir &canonical_rsv,
                                               const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] IIReservoir combine_spatial(IIReservoir cur_rsv, const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] IIReservoir spatial_reuse(IIReservoir rsv, const OCSurfaceData &cur_surf,
                                            const Int2 &pixel) const noexcept;
    [[nodiscard]] Float3 shading(IIReservoir rsv, const OCSurfaceData &cur_surf) const noexcept;

    [[nodiscard]] Bool is_neighbor(const OCSurfaceData &cur_surface,
                                   const OCSurfaceData &another_surface) const noexcept {
        return vision::is_neighbor(cur_surface, another_surface,
                                   _spatial.dot_threshold,
                                   _spatial.depth_threshold);
    }
    [[nodiscard]] Bool is_temporal_valid(const OCSurfaceData &cur_surface,
                                         const OCSurfaceData &prev_surface) const noexcept {
        return vision::is_neighbor(cur_surface, prev_surface,
                                   _temporal.dot_threshold,
                                   _temporal.depth_threshold);
    }
    [[nodiscard]] uint reservoir_base() const noexcept { return _reservoirs.index().hv(); }
    [[nodiscard]] auto prev_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().prev_surfaces_index(frame_index()));
    }
    [[nodiscard]] auto cur_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().cur_surfaces_index(frame_index()));
    }
    [[nodiscard]] auto prev_reservoirs() const noexcept {
        return pipeline()->buffer_var<ReSTIRIndirect::Reservoir>((frame_index() & 1) + reservoir_base());
    }
    [[nodiscard]] auto cur_reservoirs() const noexcept {
        return pipeline()->buffer_var<ReSTIRIndirect::Reservoir>(((frame_index() + 1) & 1) + reservoir_base());
    }
    [[nodiscard]] auto passthrough_reservoirs() const noexcept {
        return pipeline()->buffer_var<ReSTIRIndirect::Reservoir>(2 + reservoir_base());
    }
    [[nodiscard]] CommandList estimate(uint frame_index) const noexcept;
};

}// namespace vision