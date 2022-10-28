//
// Created by Zero on 28/10/2022.
//

#include "material.h"

namespace vision {

Float BSDF::PDF_(Float3 wo, Float3 wi,Uchar flag) const noexcept {
    return ocarina::Float();
}
Float3 BSDF::eval_(Float3 wo, Float3 wi,Uchar flag) const noexcept {
    return ocarina::Float3();
}
BSDFSample BSDF::sample_(Float3 wo, Float uc, Float2 u,Uchar flag) const noexcept {
    return BSDFSample();
}
}