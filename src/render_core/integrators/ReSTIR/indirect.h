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

class ReSTIRIndirectIllumination : public SerialObject, public Ctx {
private:
    uint M{};
    bool _mis{};

    SpatialResamplingParam _spatial;
    TemporalResamplingParam _temporal;

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
    [[nodiscard]] CommandList estimate(uint frame_index) const noexcept;
};

}// namespace vision