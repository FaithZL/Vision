//
// Created by Zero on 05/12/2022.
//

#include "interaction.h"
#include "base/scattering/medium.h"
#include "base/sampler.h"

namespace vision {
using namespace ocarina;

Float HenyeyGreenstein::f(Float3 wo, Float3 wi) const noexcept {
    return phase_HG(dot(wo, wi), _g);
}

PhaseSample HenyeyGreenstein::sample(Float3 wo, Sampler *sampler) const noexcept {
    Float2 u = sampler->next_2d();
    Float sqr_term = (1 - sqr(_g)) / (1 + _g - 2 * _g * u.x);
    Float cos_theta = -(1 + sqr(_g) - sqr(sqr_term)) / (2 * _g);
    cos_theta = select(abs(_g) < 1e-3f, 1 - 2 * u.x, cos_theta);

    Float sin_theta = safe_sqrt(1 - sqr(cos_theta));
    Float phi = 2 * Pi * u.y;
    Float3 v1, v2;
    coordinate_system(wo, v1, v2);
    Float3 wi = spherical_direction(sin_theta, cos_theta, phi, v1, v2, wo);
    Float f = phase_HG(cos_theta, _g);
    PhaseSample phase_sample{_swl->dimension()};
    phase_sample.eval = {{_swl->dimension(), f}, f};
    phase_sample.wi = wi;
    return phase_sample;
}

Interaction::Interaction() {}

Interaction::Interaction(Float3 pos, Float3 wo)
    : pos(pos), wo(wo) {}

void Interaction::init_phase(Float g, const SampledWavelengths &swl) {
    phase.init(g, swl);
}

Bool Interaction::has_phase() {
    return phase.valid();
}

RayState Interaction::spawn_ray_state(const Float3 &dir) const noexcept {
    OCRay ray = vision::spawn_ray(pos, g_uvn.normal(), dir);
    Uint medium = select(dot(g_uvn.normal(), dir) > 0, mi.outside, mi.inside);
    return {.ray = ray, .ior = 1.f, .medium = medium};
}

RayState Interaction::spawn_ray_state_to(const Float3 &p) const noexcept {
    OCRay ray = vision::spawn_ray_to(pos, g_uvn.normal(), p);
    Uint medium = select(dot(g_uvn.normal(), ray->direction()) > 0, mi.outside, mi.inside);
    return {.ray = ray, .ior = 1.f, .medium = medium};
}

void Interaction::set_medium(const Uint &inside, const Uint &outside) {
    mi.inside = inside;
    mi.outside = outside;
}

}// namespace vision