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

class ReSTIRIndirectIllumination : public SerialObject, public Ctx {
private:
    SpatialResamplingParam _spatial;
    TemporalResamplingParam _temporal;
    bool _open{true};
    RayTracingIntegrator *_integrator{};

    RegistrableBuffer<ReSTIRIndirect::Reservoir> _reservoirs{pipeline()->bindless_array()};
    RegistrableBuffer<ReSTIRIndirect::RSVSample> _samples{pipeline()->bindless_array()};
    optional<Uint> _frame_index;


    /**
     * initial sample
     */
    std::shared_future<Shader<void(uint)>> _initial_samples;
    /**
     * initial samples and temporal reuse
     */
    std::shared_future<Shader<void(uint)>> _temporal_reuse;
    /**
     * spatial reuse and shading
     */
    std::shared_future<Shader<void(uint)>> _spatial_shading;

protected:
    [[nodiscard]] static Sampler *sampler() noexcept { return scene().sampler(); }

public:
    ReSTIRIndirectIllumination(RayTracingIntegrator *integrator, const ParameterSet &desc);
    void prepare() noexcept;
    void compile_initial_samples() noexcept;
    void compile_temporal_reuse() noexcept;
    void compile_spatial_shading() noexcept;
    void compile() noexcept {
        compile_initial_samples();
        compile_temporal_reuse();
        compile_spatial_shading();
    }
    [[nodiscard]] Float Jacobian_det(Float3 cur_pos, Float3 neighbor_pos, Var<SurfacePoint> sample_point) const noexcept;
    [[nodiscard]] IIRSVSample init_sample(const Interaction &it, const SensorSample &ss,
                                          const OCHitContext &hit_context,
                                          SampledWavelengths &swl) noexcept;
    [[nodiscard]] IIReservoir combine_temporal(const IIReservoir &cur_rsv, OCSurfaceData cur_surf,
                                               const IIReservoir &other_rsv, SampledWavelengths &swl) const noexcept;
    [[nodiscard]] IIReservoir temporal_reuse(IIReservoir rsv, const OCSurfaceData &cur_surf, const Float2 &motion_vec,
                                             const SensorSample &ss, SampledWavelengths &swl) const noexcept;
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
    [[nodiscard]] uint surface_base() const noexcept { return _integrator->surfaces().index().hv(); }
    [[nodiscard]] uint reservoir_base() const noexcept { return _reservoirs.index().hv(); }
    [[nodiscard]] BindlessArrayBuffer<SurfaceData> prev_surfaces() const noexcept {
        return pipeline()->buffer<SurfaceData>((_frame_index.value() & 1) + surface_base());
    }
    [[nodiscard]] BindlessArrayBuffer<SurfaceData> cur_surfaces() const noexcept {
        return pipeline()->buffer<SurfaceData>(((_frame_index.value() + 1) & 1) + surface_base());
    }
    [[nodiscard]] BindlessArrayBuffer<ReSTIRIndirect::Reservoir> prev_reservoirs() const noexcept {
        return pipeline()->buffer<ReSTIRIndirect::Reservoir>((_frame_index.value() & 1) + reservoir_base());
    }
    [[nodiscard]] BindlessArrayBuffer<ReSTIRIndirect::Reservoir> cur_reservoirs() const noexcept {
        return pipeline()->buffer<ReSTIRIndirect::Reservoir>(((_frame_index.value() + 1) & 1) + reservoir_base());
    }
    [[nodiscard]] CommandList estimate(uint frame_index) const noexcept;
};

}// namespace vision