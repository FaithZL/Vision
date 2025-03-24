//
// Created by Zero on 28/10/2022.
//

#include "material.h"
#include "base/sampler.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

OC_MAKE_INSTANCE_FUNC_DEF_WITH_HOTFIX(MaterialLut, s_material_lut)

void MaterialLut::load_lut(const string &name, uint3 res,
                           PixelStorage storage, const void *data) noexcept {
    if (lut_map_.contains(name)) {
        return;
    }
    Pipeline *ppl = Global::instance().pipeline();
    RegistrableTexture texture{ppl->bindless_array()};
    texture.device_tex() = ppl->device().create_texture(res,
                                                        storage,
                                                        name);
    texture.device_tex().upload_immediately(data);
    texture.register_self();
    lut_map_.insert(std::make_pair(name, std::move(texture)));
}

void MaterialLut::unload_lut(const std::string &name) noexcept {
    if (lut_map_.contains(name)) {
        lut_map_.erase(name);
    }
}

const RegistrableTexture &MaterialLut::get_lut(const std::string &name) const noexcept {
    OC_ASSERT(lut_map_.contains(name));
    return lut_map_.at(name);
}

const EncodedData<uint> &MaterialLut::get_index(const std::string &name) const noexcept {
    return lut_map_.at(name).index();
}

///#region MaterialEvaluator
ScatterEval MaterialEvaluator::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              MaterialEvalMode mode,
                                              const Uint &flag,
                                              TransportMode tm) const noexcept {
    ScatterEval ret{*swl_};
    dispatch([&](const Lobe *lobe_set) {
        ret = lobe_set->evaluate_local(wo, wi, mode, flag, tm);
    });
    return ret;
}

BSDFSample MaterialEvaluator::sample_local(const Float3 &wo, const Uint &flag,
                                           TSampler &sampler,
                                           TransportMode tm) const noexcept {
    BSDFSample ret{*swl_};
    dispatch([&](const Lobe *lobe_set) {
        ret = lobe_set->sample_local(wo, flag, sampler, tm);
    });
    return ret;
}

BSDFSample MaterialEvaluator::sample_delta_local(const Float3 &wo,
                                                 TSampler &sampler) const noexcept {
    BSDFSample ret{*swl_};
    dispatch([&](const Lobe *lobe_set) {
        ret = lobe_set->sample_delta_local(wo, sampler);
    });
    return ret;
}

void MaterialEvaluator::regularize() noexcept {
    dispatch([&](Lobe *lobe_set) {
        lobe_set->regularize();
    });
}

void MaterialEvaluator::mollify() noexcept {
    dispatch([&](Lobe *lobe_set) {
        lobe_set->mollify();
    });
}

SampledSpectrum MaterialEvaluator::albedo(const Float3 &world_wo) const noexcept {
    SampledSpectrum ret{swl_->dimension()};
    Float cos_theta = dot(shading_frame_.normal(), world_wo);
    dispatch([&](const Lobe *lobe_set) {
        ret = lobe_set->albedo(cos_theta);
    });
    return ret;
}

Bool MaterialEvaluator::splittable() const noexcept {
    Bool ret = false;
    dispatch([&](const Lobe *lobe_set) {
        ret = lobe_set->splittable();
    });
    return ret;
}

optional<Bool> MaterialEvaluator::is_dispersive() const noexcept {
    optional<Bool> ret;
    dispatch([&](const Lobe *lobe_set) {
        ret = lobe_set->is_dispersive();
    });
    return ret;
}

Uint MaterialEvaluator::flag() const noexcept {
    Uint ret;
    dispatch([&](const Lobe *lobe_set) {
        ret = lobe_set->flag();
    });
    return ret;
}

ScatterEval MaterialEvaluator::evaluate(const Float3 &world_wo, const Float3 &world_wi,
                                        MaterialEvalMode mode, const Uint &flag,
                                        TransportMode tm) const noexcept {
    Float3 wo = shading_frame_.to_local(world_wo);
    Float3 wi = shading_frame_.to_local(world_wi);
    ScatterEval ret = evaluate_local(wo, wi, mode, flag, tm);
    Bool discard = same_hemisphere(world_wo, world_wi, ng_) == BxDFFlag::is_transmission(ret.flags);
    ret.pdfs = select(discard, 0.f, ret.pdfs);
    ret.f *= abs_cos_theta(wi);
    return ret;
}

BSDFSample MaterialEvaluator::sample_delta(const Float3 &world_wo,
                                           TSampler &sampler) const noexcept {
    Float3 wo = shading_frame_.to_local(world_wo);
    BSDFSample ret = sample_delta_local(wo, sampler);
    ret.eval.f *= abs_cos_theta(ret.wi);
    ret.wi = shading_frame_.to_world(ret.wi);
    return ret;
}

BSDFSample MaterialEvaluator::sample(const Float3 &world_wo, TSampler &sampler,
                                     const Uint &flag,
                                     TransportMode tm) const noexcept {
    Float3 wo = shading_frame_.to_local(world_wo);
    BSDFSample ret = sample_local(wo, flag, sampler, tm);
    ret.eval.f *= abs_cos_theta(ret.wi);
    ret.wi = shading_frame_.to_world(ret.wi);
    Bool discard = same_hemisphere(world_wo, ret.wi, ng_) == BxDFFlag::is_transmission(ret.eval.flags);
    ret.eval.pdfs = select(discard, 0.f, ret.eval.pdfs);
    return ret;
}
///#endregion

Material::Material(const vision::MaterialDesc &desc) : Node(desc) {
    if (desc.has_attr("bump")) {
        bump_.set(Slot::create_slot(desc.slot("bump", 1.f, Number)));
        bump_scale_.set(Slot::create_slot(desc.slot("bump_scale", 1.f, Number)));
    }
}

TSampler &Material::get_sampler() noexcept {
    return scene().sampler();
}

bool Material::render_UI(ocarina::Widgets *widgets) noexcept {
    string label = format("{} {} material: {}, type_index: {}",
                          index_, impl_type().data(),
                          name_.c_str(), MaterialRegistry::instance().materials().type_index(this));
    bool open = widgets->use_tree(label, [&] {
        render_sub_UI(widgets);
    });
    return open;
}

void Material::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    widgets->drag_uint("exp of 2", addressof(exp_of_two_), 1, 1, 20);
    for_each_slot([&](Slot &slot) {
        slot.render_UI(widgets);
    });
}

void Material::restore(vision::RuntimeObject *old_obj) noexcept {
    Node::restore(old_obj);
    VS_HOTFIX_MOVE_ATTRS(index_, slot_cursor_, bump_, bump_scale_, exp_of_two_)
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

///#region encodable
uint Material::compacted_size() const noexcept {
    return reduce_slots(0u, [&](uint size, const Slot &slot) {
        return size + slot->compacted_size();
    });
}

uint Material::cal_offset(ocarina::uint prev_size) const noexcept {
    return reduce_slots(prev_size, [&](uint size, const Slot &slot) {
        return slot->cal_offset(size);
    });
}

uint Material::alignment() const noexcept {
    return reduce_slots(0u, [&](uint align, const Slot &slot) {
        return ocarina::max(align, slot->alignment());
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

void Material::invalidate() const noexcept {
    for_each_slot([&](const Slot &slot) {
        slot->invalidate();
    });
}

void Material::encode(RegistrableManaged<buffer_ty> &data) const noexcept {
    for_each_slot([&](const Slot &slot) {
        slot->encode(data);
    });
}

void Material::decode(const DataAccessor *da) const noexcept {
    for_each_slot([&](const Slot &slot) {
        slot->decode(da);
    });
}

void Material::decode(const DynamicArray<ocarina::buffer_ty> &array) const noexcept {
    for_each_slot([&](const Slot &slot) {
        slot->decode(array);
    });
}

///#endregion

void Material::reset_status() noexcept {
    for_each_slot([&](Slot &slot) {
        slot.reset_status();
    });
    Node::reset_status();
}

bool Material::has_changed() noexcept {
    return Node::has_changed() || reduce_slots(false, [&](bool b, Slot &slot) {
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

void compute_by_normal_map(const Slot &normal_map, const Slot &scale,
                           Interaction *it,
                           const SampledWavelengths &swl) noexcept {
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

void compute_by_bump_map(const Slot &bump_map, const Slot &scale,
                         Interaction *it,
                         const SampledWavelengths &swl) noexcept {
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

SampledSpectrum Material::integral_albedo(const Float3 &wo, const Lobe *lobe_set) const noexcept {
    TSampler &sampler = scene().sampler();
    uint sample_num = 2 << exp_of_two_;
    return lobe_set->integral_albedo(wo, sampler, sample_num);
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

string PrecomputedLobeTable::to_string() const noexcept {
    uint dim = type->dimension();
    size_t count = data.size() / type->dimension();
    std::ostringstream content;
    uint line_len = res.x;

    uint area = res.x * res.y;

    auto func = [&]<typename T>(T t) {
        for (uint i = 0; i < data.size(); i += dim) {
            T elm = T(addressof(data[i]));
            content << (i % (line_len * dim) == 0 ? "\n\t" : "");
            content << to_str(elm) << ((i / dim) == (data.size() / dim - 1) ? "\n" : ", ");
            if ((i / dim + 1) % (area) == 0) {
                content << endl;
            }
        }
    };

    if (type->name() == "float") {
        for (int i = 0; i < data.size(); ++i) {
            content << (i % line_len == 0 ? "\n\t" : "");
            content << std::to_string(data[i]) << (i == data.size() - 1 ? "\n" : ", ");
            if ((i + 1) % area == 0) {
                content << endl;
            }
        }
    } else if (type->name() == "float2") {
        func(float2{});
    } else if (type->name() == "float4") {
        func(float4{});
    } else {
        OC_ASSERT(false);
    }

    string str = ocarina::format("/// it took {:.3f} s \nconst {} {}_Table[{}] = {{{}}};",
                                 elapsed_time, type->name(),
                                 name, count, content.str());

    return str;
}

vector<PrecomputedLobeTable> Material::precompute() const noexcept {
    return {};
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