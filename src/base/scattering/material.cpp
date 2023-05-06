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

ScatterEval BSDF::evaluate(Float3 world_wo, Float3 world_wi) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    Float3 wi = shading_frame.to_local(world_wi);
    ScatterEval ret = evaluate_local(wo, wi, BxDFFlag::All);
    ret.f *= abs_cos_theta(wi);
    return ret;
}

BSDFSample BSDF::sample(Float3 world_wo, Sampler *sampler) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    BSDFSample ret = sample_local(wo, BxDFFlag::All, sampler);
    ret.eval.f *= abs_cos_theta(ret.wi);
    ret.wi = shading_frame.to_world(ret.wi);
    return ret;
}

ScatterEval DielectricBxDFSet::evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept {
    ScatterEval ret{_refl.swl().dimension()};
    auto fresnel = _fresnel->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    $if(same_hemisphere(wo, wi)) {
        ret = _refl.evaluate(wo, wi, fresnel);
    }
    $else {
        ret = _trans.evaluate(wo, wi, fresnel);
    };
    return ret;
}

BSDFSample DielectricBxDFSet::sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept {
    BSDFSample ret{_refl.swl().dimension()};
    Float uc = sampler->next_1d();
    auto fresnel = _fresnel->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
    $if(uc < fr) {
        ret = _refl.sample(wo, sampler, fresnel);
        ret.eval.pdf *= fr;
    }
    $else {
        ret = _trans.sample(wo, sampler, fresnel);
        ret.eval.pdf *= 1 - fr;
    };
    return ret;
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

void Material::encode(ManagedWrapper<float> &data) const noexcept {
    for_each_slot([&](const Slot &slot) {
        slot->encode(data);
    });
}

void Material::decode(const DataAccessor<float> *da) const noexcept {
    for_each_slot([&](const Slot &slot) {
        slot->decode(da);
    });
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