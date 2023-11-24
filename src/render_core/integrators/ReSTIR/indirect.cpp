//
// Created by Zero on 2023/10/29.
//

#include "indirect.h"

namespace vision {

ReSTIRIndirectIllumination::ReSTIRIndirectIllumination(const vision::ParameterSet &desc,
                                                       RegistrableManaged<float2> &motion_vec,
                                                       RegistrableManaged<SurfaceData> &surfaces,
                                                       RegistrableManaged<SurfaceData> &prev_surfaces)
    : _motion_vectors(motion_vec),
      _surfaces(surfaces),
      _prev_surfaces(prev_surfaces) {
}

}// namespace vision