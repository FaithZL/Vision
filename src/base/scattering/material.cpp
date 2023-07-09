//
// Created by Zero on 28/10/2022.
//

#include "material.h"
#include "base/sampler.h"
#include "base/mgr/scene.h"

namespace vision {

Uint BSDF::combine_flag(Float3 wo, Float3 wi, Uint flag) noexcept {
    Bool reflect = same_hemisphere(wo, wi);
    Uint non_reflect = ~BxDFFlag::Reflection;
    Uint non_trans = ~BxDFFlag::Transmission;
    return select(reflect, flag & non_trans, flag & non_reflect);
}

ScatterEval BSDF::evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept {
    ScatterEval ret = bxdf_set->evaluate_local(wo, wi, flag);
    return ret;
}

BSDFSample BSDF::sample_local(ocarina::Float3 wo, ocarina::Uint flag, vision::Sampler *sampler) const noexcept {
    BSDFSample ret = bxdf_set->sample_local(wo, flag, sampler);
    return ret;
}

ScatterEval BSDF::evaluate(Float3 world_wo, Float3 world_wi) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    Float3 wi = shading_frame.to_local(world_wi);
    ScatterEval ret = evaluate_local(wo, wi, BxDFFlag::All);
    Bool discard = same_hemisphere(world_wo, world_wi, ng) == BxDFFlag::is_transmission(ret.flags);
    ret.pdf = select(discard, 0.f, ret.pdf);
    ret.f *= abs_cos_theta(wi);
    return ret;
}

BSDFSample BSDF::sample(Float3 world_wo, Sampler *sampler) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    BSDFSample ret = sample_local(wo, BxDFFlag::All, sampler);
    ret.eval.f *= abs_cos_theta(ret.wi);
    ret.wi = shading_frame.to_world(ret.wi);
    Bool discard = same_hemisphere(world_wo, ret.wi, ng) == BxDFFlag::is_transmission(ret.eval.flags);
    ret.eval.pdf = select(discard, 0.f, ret.eval.pdf);
    return ret;
}

Material::Material(const vision::MaterialDesc &desc) : Node(desc) {
    if (desc.has_attr("bump")) {
        _bump = scene().create_slot(desc.slot("bump", 1.f, Number));
        _bump_scale = scene().create_slot(desc.slot("bump_scale", 1.f, Number));
    }
}

uint Material::element_num() const noexcept {
    return reduce_slots(0u, [&](uint size, const Slot &slot) {
        return size + slot->element_num();
    });
}

bool Material::has_device_value() const noexcept {
    return reduce_slots(true, [&](bool b, const Slot &slot) {
        return b && slot->has_device_value();
    });
}

void Material::reset_device_value() const noexcept {
    for_each_slot([&](const Slot &slot) {
        slot->reset_device_value();
    });
}

void Material::encode(RegistrableManaged<float> &data) const noexcept {
    for_each_slot([&](const Slot &slot) {
        slot->encode(data);
    });
}

void Material::decode(const DataAccessor<float> *da) const noexcept {
    for_each_slot([&](const Slot &slot) {
        slot->decode(da);
    });
}

namespace detail {

[[nodiscard]] Float3 clamp_ns(Float3 ns, Float3 ng, Float3 w) {
    Float3 w_refl = reflect(w, ns);
    Float3 w_refl_clip = select(same_hemisphere(w, w_refl, ng), w_refl,
                                normalize(w_refl - ng * dot(w_refl, ng)));
    return normalize(w_refl_clip + w);
}

void compute_by_normal_map(const Slot &normal_map, const Slot &scale, Interaction *it, const SampledWavelengths &swl) noexcept {
    Float3 normal = normal_map.evaluate(*it, swl).as_vec3() * 2.f - make_float3(1.f);
    Float s = scale.evaluate(*it, swl).as_scalar();
    normal.x *= s;
    normal.y *= s;
    Float3 world_normal = it->s_uvn.to_world(normal);
    world_normal = normalize(world_normal);
    world_normal = clamp_ns(world_normal, it->g_uvn.normal(), it->wo);
    world_normal = normalize(face_forward(world_normal, it->s_uvn.normal()));
    it->s_uvn.z = world_normal;
    it->s_uvn.x = normalize(cross(world_normal, it->s_uvn.y));
    it->s_uvn.y = cross(world_normal, it->s_uvn.x);
}

void compute_by_bump_map(const Slot &bump_map, const Slot &scale, Interaction *it, const SampledWavelengths &swl) noexcept {
}
}// namespace detail

void Material::_apply_bump(Interaction *it, const SampledWavelengths &swl) const noexcept {
    switch (_bump.dim()) {
        case 1: detail::compute_by_bump_map(_bump, _bump_scale, it, swl); break;
        case 3: detail::compute_by_normal_map(_bump, _bump_scale, it, swl); break;
        default: break;
    }
}

BSDF Material::compute_BSDF(Interaction it, const SampledWavelengths &swl) const noexcept {
    if (_bump) {
        _apply_bump(&it, swl);
    }
    return _compute_BSDF(it, swl);
}

uint64_t Material::_compute_type_hash() const noexcept {
    uint64_t ret = Hash64::default_seed;
    reduce_slots(ret, [&](uint64_t hash, const Slot &slot) {
        return hash64(hash, slot.type_hash());
    });
    return ret;
}

uint64_t Material::_compute_hash() const noexcept {
    uint64_t ret = Hash64::default_seed;
    reduce_slots(ret, [&](uint64_t hash, const Slot &slot) {
        return hash64(hash, slot.hash());
    });
    return ret;
}

}// namespace vision