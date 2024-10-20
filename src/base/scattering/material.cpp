//
// Created by Zero on 28/10/2022.
//

#include "material.h"
#include "base/sampler.h"
#include "base/mgr/scene.h"

namespace vision {

ScatterEval MaterialEvaluator::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              MaterialEvalMode mode, const Uint &flag) const noexcept {
    ScatterEval ret{*swl_};
    dispatch([&](const BxDFSet *lobe_set) {
        ret = lobe_set->evaluate_local(wo, wi, mode, flag);
    });
    return ret;
}

BSDFSample MaterialEvaluator::sample_local(const Float3 &wo, const Uint &flag,
                                           TSampler &sampler) const noexcept {
    BSDFSample ret{*swl_};
    dispatch([&](const BxDFSet *lobe_set) {
        ret = lobe_set->sample_local(wo, flag, sampler);
    });
    return ret;
}

BSDFSample MaterialEvaluator::sample_delta_local(const Float3 &wo, TSampler &sampler) const noexcept {
    BSDFSample ret{*swl_};
    dispatch([&](const BxDFSet *lobe_set) {
        ret = lobe_set->sample_delta_local(wo, sampler);
    });
    return ret;
}

void MaterialEvaluator::regularize() noexcept {
    dispatch([&](BxDFSet *lobe_set) {
        lobe_set->regularize();
    });
}

void MaterialEvaluator::mollify() noexcept {
    dispatch([&](BxDFSet *lobe_set) {
        lobe_set->mollify();
    });
}

SampledSpectrum MaterialEvaluator::albedo(const Float3 &world_wo) const noexcept {
    SampledSpectrum ret{swl_->dimension()};
    Float3 wo = shading_frame_.to_local(world_wo);
    dispatch([&](const BxDFSet *lobe_set) {
        ret = lobe_set->albedo(wo);
    });
    return ret;
}

Bool MaterialEvaluator::splittable() const noexcept {
    Bool ret = false;
    dispatch([&](const BxDFSet *lobe_set) {
        ret = lobe_set->splittable();
    });
    return ret;
}

optional<Bool> MaterialEvaluator::is_dispersive() const noexcept {
    optional<Bool> ret;
    dispatch([&](const BxDFSet *lobe_set) {
        ret = lobe_set->is_dispersive();
    });
    return ret;
}

Uint MaterialEvaluator::flag() const noexcept {
    Uint ret;
    dispatch([&](const BxDFSet *lobe_set) {
        ret = lobe_set->flag();
    });
    return ret;
}

ScatterEval MaterialEvaluator::evaluate(const Float3 &world_wo, const Float3 &world_wi,
                                        MaterialEvalMode mode,
                                        const Uint &flag) const noexcept {
    Float3 wo = shading_frame_.to_local(world_wo);
    Float3 wi = shading_frame_.to_local(world_wi);
    ScatterEval ret = evaluate_local(wo, wi, mode, flag);
    Bool discard = same_hemisphere(world_wo, world_wi, ng_) == BxDFFlag::is_transmission(ret.flags);
    ret.pdfs = select(discard, 0.f, ret.pdfs);
    ret.f *= abs_cos_theta(wi);
    return ret;
}

BSDFSample MaterialEvaluator::sample_delta(const Float3 &world_wo, TSampler &sampler) const noexcept {
    Float3 wo = shading_frame_.to_local(world_wo);
    BSDFSample ret = sample_delta_local(wo, sampler);
    ret.eval.f *= abs_cos_theta(ret.wi);
    ret.wi = shading_frame_.to_world(ret.wi);
    return ret;
}

BSDFSample MaterialEvaluator::sample(const Float3 &world_wo, TSampler &sampler, const Uint &flag) const noexcept {
    Float3 wo = shading_frame_.to_local(world_wo);
    BSDFSample ret = sample_local(wo, flag, sampler);
    ret.eval.f *= abs_cos_theta(ret.wi);
    ret.wi = shading_frame_.to_world(ret.wi);
    Bool discard = same_hemisphere(world_wo, ret.wi, ng_) == BxDFFlag::is_transmission(ret.eval.flags);
    ret.eval.pdfs = select(discard, 0.f, ret.eval.pdfs);
    return ret;
}

Material::Material(const vision::MaterialDesc &desc) : Node(desc) {
    if (desc.has_attr("bump")) {
        bump_.set(Slot::create_slot(desc.slot("bump", 1.f, Number)));
        bump_scale_.set(Slot::create_slot(desc.slot("bump_scale", 1.f, Number)));
    }
}

bool Material::render_UI(ocarina::Widgets *widgets) noexcept {
    string label = format("{} {} material: {}", index_, impl_type().data(), name_.c_str());
    bool open = widgets->use_tree(label, [&] {
        render_sub_UI(widgets);
    });
    return open;
}

void Material::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    for_each_slot([&](Slot &slot) {
        slot.render_UI(widgets);
    });
}

void Material::restore(vision::RuntimeObject *old_obj) noexcept {
    Node::restore(old_obj);
    VS_HOTFIX_MOVE_ATTRS(index_, slot_cursor_, bump_, bump_scale_)
    for (int i = 0; i < slot_cursor_.num; ++i) {
        Slot &slot = get_slot(i);
        Slot &old_slot = old_obj_->get_slot(i);
        slot = ocarina::move(old_slot);
    }
    for (const auto &item : old_obj_->shape_instances) {
        auto sp = item.lock();
        sp->set_material(shared_from_this());
        sp->set_material_name(name());
    }
    for (const auto &item : old_obj_->shape_groups) {
        auto sp = item.lock();
        sp->set_material(shared_from_this());
        sp->set_material_name(name());
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

void Material::reset_status() noexcept {
    for_each_slot([&](Slot &slot) {
        slot.reset_status();
    });
}

bool Material::has_changed() noexcept {
    return reduce_slots(false, [&](bool b, Slot &slot) {
        return b || slot->has_changed();
    });
}

namespace detail {

[[nodiscard]] Float3 clamp_ns(Float3 ns, Float3 ng, const Float3 &w) {
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
    Float3 world_normal = it->shading.to_world(normal);
    world_normal = normalize(world_normal);
    world_normal = clamp_ns(world_normal, it->ng, it->wo);
    world_normal = normalize(face_forward(world_normal, it->shading.normal()));
    it->shading.update(world_normal);
}

void compute_by_bump_map(const Slot &bump_map, const Slot &scale, Interaction *it, const SampledWavelengths &swl) noexcept {
    static constexpr float d = 0.0005f;

    Interaction it_eval = *it;

    Float du = 0.5f * (abs(it->du_dx) + abs(it->du_dy));
    du = select(du == 0.f, d, du);
    it_eval.pos = it->pos + du * it->shading.dp_du();
    it_eval.uv = it->uv + make_float2(du, 0.f);
    it_eval.ng = normalize(cross(it->shading.dp_du(), it->shading.dp_dv()));
    Float u_displace = bump_map.evaluate(it_eval, swl).as_scalar();

    Float dv = 0.5f * (abs(it->dv_dx) + abs(it->dv_dy));
    dv = select(dv == 0.f, d, dv);
    it_eval.pos = it->pos + dv * it->shading.dp_dv();
    it_eval.uv = it->uv + make_float2(0.f, dv);
    it_eval.ng = normalize(cross(it->shading.dp_du(), it->shading.dp_dv()));
    Float v_displace = bump_map.evaluate(it_eval, swl).as_scalar();

    Float displace = bump_map.evaluate(*it, swl).as_scalar();

    Float3 dp_du = it->shading.dp_du() +
                   (u_displace - displace) / du * it->shading.normal() +
                   displace * it->shading.dn_du;

    Float3 dp_dv = it->shading.dp_dv() +
                   (v_displace - displace) / dv * it->shading.normal() +
                   displace * it->shading.dn_dv;

    it->shading.z = normalize(cross(dp_du, dp_dv));
    it->shading.set(dp_du, dp_dv, normalize(cross(dp_du, dp_dv)));
}
}// namespace detail

Uint Material::combine_flag(const Float3 &wo, const Float3 &wi, Uint flag) noexcept {
    Bool reflect = same_hemisphere(wo, wi);
    Uint non_reflect = ~BxDFFlag::Reflection;
    Uint non_trans = ~BxDFFlag::Transmission;
    return select(reflect, flag & non_trans, flag & non_reflect);
}

void Material::_apply_bump(Interaction *it, const SampledWavelengths &swl) const noexcept {
    switch (bump_.dim()) {
        case 1: detail::compute_by_bump_map(bump_, bump_scale_, it, swl); break;
        case 3: detail::compute_by_normal_map(bump_, bump_scale_, it, swl); break;
        default: break;
    }
}

MaterialEvaluator Material::create_evaluator(const Interaction &it,
                                             const SampledWavelengths &swl) const noexcept {
    MaterialEvaluator evaluator{it, swl};
    build_evaluator(evaluator, it, swl);
    return evaluator;
}

void Material::build_evaluator(Evaluator &evaluator, Interaction it,
                               const SampledWavelengths &swl) const noexcept {
    if (bump_) {
        _apply_bump(std::addressof(it), swl);
    }
    _build_evaluator(evaluator, it, swl);
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