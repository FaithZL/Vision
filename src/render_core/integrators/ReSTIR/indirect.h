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

class ReSTIRIndirectIllumination : public SerialObject, public Ctx {
private:
    uint M{};
    bool _mis{};

    SpatialResamplingParam _spatial;
    TemporalResamplingParam _temporal;

    RegistrableBuffer<ReSTIRIndirect::Reservoir> _reservoirs{pipeline()->bindless_array()};
    RegistrableBuffer<ReSTIRIndirect::RSVSample> _init_samples{pipeline()->bindless_array()};
    RegistrableBuffer<SurfaceData> &_surfaces;
    RegistrableBuffer<float2> &_motion_vectors;
    RegistrableBuffer<Ray> &_rays;

public:
    ReSTIRIndirectIllumination(const ParameterSet &desc, RegistrableBuffer<float2> &motion_vec,
                               RegistrableBuffer<SurfaceData> &surfaces,
                               RegistrableBuffer<Ray> &rays);

    void prepare() noexcept;
};

}// namespace vision