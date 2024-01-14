//
// Created by Zero on 2023/9/11.
//

#pragma once

#include "common.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"
#include "indirect_util.h"

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

public:
    ReSTIRIndirectIllumination(RayTracingIntegrator *integrator, const ParameterSet &desc);

    void prepare() noexcept;
    [[nodiscard]] CommandList estimate(uint frame_index) const noexcept;
};

}// namespace vision