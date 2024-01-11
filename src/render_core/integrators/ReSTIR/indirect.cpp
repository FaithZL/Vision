//
// Created by Zero on 2023/10/29.
//

#include "indirect.h"

namespace vision {

ReSTIRIndirectIllumination::ReSTIRIndirectIllumination(const vision::ParameterSet &desc,
                                                       RegistrableBuffer<float2> &motion_vec,
                                                       RegistrableBuffer<SurfaceData> &surfaces)
    : _motion_vectors(motion_vec),
      _surfaces(surfaces) {
}

}// namespace vision