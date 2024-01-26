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
    RegistrableBuffer<ReSTIRIndirect::RSVSample> _init_samples{pipeline()->bindless_array()};
    optional<Uint> _frame_index;

    /**
     * initial samples and temporal reuse
     */
    std::shared_future<Shader<void(uint)>> _shader0;
    /**
     * spatial reuse and shading
     */
    std::shared_future<Shader<void(uint)>> _shader1;

protected:
    [[nodiscard]] static Sampler *sampler() noexcept { return scene().sampler(); }

public:
    ReSTIRIndirectIllumination(RayTracingIntegrator *integrator, const ParameterSet &desc);
    void prepare() noexcept;
    void compile_shader0() noexcept;
    void compile_shader1() noexcept;
    void compile() noexcept {
        compile_shader0();
        compile_shader1();
    }
    void init_sample(const Interaction &it, SampledWavelengths &swl) noexcept;
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