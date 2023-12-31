//
// Created by Zero on 2023/9/11.
//

#pragma once

#include "common.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"
#include "indirect_util.h"

namespace vision {

class ReSTIRIndirectIllumination : public SerialObject, public Ctx {
private:
    uint M{};
    bool _mis{};

    SpatialResamplingParam _spatial;
    TemporalResamplingParam _temporal;

    RegistrableBuffer<SurfaceData> &_surfaces;
    RegistrableBuffer<SurfaceData> &_prev_surfaces;
    RegistrableBuffer<float2> &_motion_vectors;

public:
    ReSTIRIndirectIllumination(const ParameterSet &desc, RegistrableBuffer<float2> &motion_vec,
                               RegistrableBuffer<SurfaceData> &surfaces,
                               RegistrableBuffer<SurfaceData> &prev_surfaces);

    void prepare() noexcept;
};

}// namespace vision