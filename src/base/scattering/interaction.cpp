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
    phase_sample.eval = {{_swl->dimension(), f}, f, 0};
    phase_sample.wi = wi;
    return phase_sample;
}

float Interaction::s_ray_offset_factor = 1.f;

float Interaction::ray_offset_factor() noexcept {
    return s_ray_offset_factor;
}

void Interaction::set_ray_offset_factor(float value) noexcept {
    s_ray_offset_factor = value;
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

Uint Interaction::material_inst_id() const noexcept {
    return decode_id<D>(_mat_id).first;
}

Uint Interaction::material_type_id() const noexcept {
    return decode_id<D>(_mat_id).second;
}

Uint Interaction::light_inst_id() const noexcept {
    return decode_id<D>(_light_id).first;
}

Uint Interaction::light_type_id() const noexcept {
    return decode_id<D>(_light_id).second;
}

OCRay Interaction::spawn_ray(const Float3 &dir) const noexcept {
    return vision::spawn_ray(pos, ng, dir);
}

OCRay Interaction::spawn_ray(const Float3 &dir, const Float &t) const noexcept {
    return vision::spawn_ray(pos, ng, dir, t);
}

RayState Interaction::spawn_ray_state(const Float3 &dir) const noexcept {
    OCRay ray = vision::spawn_ray(pos, ng, dir);
    Uint medium = select(dot(ng, dir) > 0, mi.outside, mi.inside);
    return {.ray = ray, .ior = 1.f, .medium = medium};
}

RayState Interaction::spawn_ray_state_to(const Float3 &p) const noexcept {
    OCRay ray = vision::spawn_ray_to(pos, ng, p);
    Uint medium = select(dot(ng, ray->direction()) > 0, mi.outside, mi.inside);
    return {.ray = ray, .ior = 1.f, .medium = medium};
}

OCRay Interaction::spawn_ray_to(const Float3 &p) const noexcept {
    return vision::spawn_ray_to(pos, ng, p);
}

void Interaction::set_medium(const Uint &inside, const Uint &outside) {
    mi.inside = inside;
    mi.outside = outside;
}

}// namespace vision