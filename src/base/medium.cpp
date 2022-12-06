//
// Created by Zero on 02/12/2022.
//

#include "medium.h"
#include "math/geometry.h"

namespace vision {

Float HenyeyGreenstein::f(Float3 wo, Float3 wi) const noexcept {
    return phase_HG(dot(wo, wi), _g);
}

PhaseSample HenyeyGreenstein::sample(Float3 wo, Float2 u) const noexcept {
    Float sqr_term = (1 - sqr(_g)) / (1 + _g - 2 * _g * u.x);
    Float cos_theta = -(1 + sqr(_g) - sqr(sqr_term)) / (1 * _g);
    cos_theta = select(abs(_g) < 1e-3f, 1 - 2 * u.x, cos_theta);

    Float sin_theta = safe_sqrt(1 - sqr(cos_theta));
    Float phi = 2 * Pi * u.y;
    Float3 v1, v2;
    coordinate_system(wo, v1, v2);
    Float3 wi = spherical_direction(sin_theta, cos_theta, phi, v1, v2, wo);
    Float f = phase_HG(cos_theta, _g);
    return {.eval = {.f = make_float3(f), .pdf = f}, .wi = wi};
}

}// namespace vision