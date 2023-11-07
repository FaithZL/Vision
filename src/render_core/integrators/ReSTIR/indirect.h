//
// Created by Zero on 2023/9/11.
//

#pragma once

#include "common.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"
#include "indirect_util.h"

namespace vision {

using namespace ReSTIRIndirect;
class ReSTIRIndirectIllumination : public SerialObject, public Ctx {
private:
    SpatialResamplingParam _spatial;
    TemporalResamplingParam _temporal;
    RSVSample s;
};

}