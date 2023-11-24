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

    RegistrableManaged<SurfaceData> &_surfaces;
    RegistrableManaged<SurfaceData> &_prev_surfaces;
    RegistrableManaged<float2> &_motion_vectors;

public:
    ReSTIRIndirectIllumination(const ParameterSet &desc, RegistrableManaged<float2> &motion_vec,
                               RegistrableManaged<SurfaceData> &surfaces,
                               RegistrableManaged<SurfaceData> &prev_surfaces);

    void prepare() noexcept;
};

}// namespace vision