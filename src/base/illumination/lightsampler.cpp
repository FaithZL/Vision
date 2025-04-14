//
// Created by Zero on 22/10/2022.
//

#include "lightsampler.h"
#include "base/mgr/pipeline.h"
#include "base/shape.h"

namespace vision {

LightSampler::LightSampler(const LightSamplerDesc &desc)
    : Node(desc),
      env_separate_(desc["env_separate"].as_bool(false)),
      env_prob_(ocarina::clamp(desc["env_prob"].as_float(0.5f), 0.01f, 0.99f)) {
    for (const LightDesc &light_desc : desc.light_descs) {
        TLight light = TLight(light_desc);
        if (light->match(LightType::Area)) {
            TObject<IAreaLight> emission = dynamic_object_cast<IAreaLight>(light);
            emission->instance()->set_emission(emission);
        }
        if (light->match(LightType::Infinite)) {
            env_light_ = dynamic_object_cast<Environment>(light);
        }
        add_light(light);
    }
}

Uint LightSampler::correct_index(Uint index) const noexcept {
    if (env_light() && env_separate_) {
        return ocarina::select(index < env_index(), index, index + 1u);
    }
    return index;
}

void LightSampler::update_runtime_object(const vision::IObjectConstructor *constructor) noexcept {
    for (int i = 0; i < lights_.size(); ++i) {
        TLight light = lights_[i];
        if (!constructor->match(light.get())) {
            continue;
        }
        auto new_light = TLight{constructor->construct_shared<Light>()};
        switch (new_light->type()) {
            case LightType::Area: {
                TObject<IAreaLight> new_al = dynamic_object_cast<IAreaLight>(new_light);
                TObject<IAreaLight> old_al = dynamic_object_cast<IAreaLight>(light);
                ShapeInstance *shape_instance = old_al->instance();
                shape_instance->set_emission(new_al);
                break;
            }
            case LightType::Infinite: {
                TObject<Environment> new_env = dynamic_object_cast<Environment>(new_light);
                TObject<Environment> old_env = dynamic_object_cast<Environment>(light);
                env_light_ = new_env;
                break;
            }
            default:
                break;
        }
        new_light->restore(light.get());
        lights_.replace(i, std::move(new_light));
    }
}

void LightSampler::tidy_up() noexcept {

    static_assert(ocarina::is_ptr_v<TObject<Filter>>);

    std::sort(lights_.begin(), lights_.end(), [&](TLight a, TLight b) {
        return lights_.type_index(a.get()) < lights_.type_index(b.get());
    });
    for_each([&](TLight light, uint index) noexcept {
        light->set_index(index);
    });
}

void LightSampler::prepare() noexcept {
    for_each([&](TLight light, uint index) noexcept {
        light->prepare();
    });
    auto rp = pipeline();
    lights_.prepare(rp->bindless_array(), rp->device());
}

void LightSampler::update_device_data() noexcept {
    if (has_changed()) {
        lights_.update();
    }
}

bool LightSampler::render_UI(ocarina::Widgets *widgets) noexcept {
    bool open = widgets->use_folding_header(
        ocarina::format("{} light sampler", impl_type().data()),
        [&] {
            widgets->check_box("env_separate", &env_separate_);
            widgets->drag_float("env_prob", &env_prob_, 0.01, 0.01, 0.99);
            render_sub_UI(widgets);
            lights_.render_UI(widgets);
        });
    return open;
}

Uint LightSampler::extract_light_index(const vision::Interaction &it) const noexcept {
    return combine_to_light_index(it.light_type_id(), it.light_inst_id());
}

Uint LightSampler::combine_to_light_index(const Uint &type_id, const Uint &inst_id) const noexcept {
    vector<uint> nums;
    Uint ret = 0u;
    switch (lights_.mode()) {
        case ocarina::EInstance: {
            ret = inst_id;
            break;
        }
        case ocarina::EType: {
            nums.reserve(lights_.type_num());
            for (int i = 0; i < lights_.type_num(); ++i) {
                nums.push_back(static_cast<uint>(lights_.instance_num(i)));
            }
            DynamicArray<uint> arr{nums};
            $for(i, type_id) {
                ret += arr[i];
            };
            ret += inst_id;
            break;
        }
        default:
            break;
    }
    return ret;
}

pair<Uint, Uint> LightSampler::extract_light_id(const Uint &index) const noexcept {
    Uint type_id = 0u;
    Uint inst_id = 0u;
    vector<uint> nums;
    nums.reserve(lights_.type_num());
    for (int i = 0; i < lights_.type_num(); ++i) {
        nums.push_back(static_cast<uint>(lights_.instance_num(i)));
    }

    Uint accum = 0u;
    for (uint i = 0; i < nums.size(); ++i) {
        type_id = select(index >= accum, i, type_id);
        inst_id = select(index >= accum, index - accum, inst_id);
        accum += nums[i];
    }
    switch (lights_.mode()) {
        case ocarina::EInstance:
            return {type_id, index};
        case ocarina::EType:
            return {type_id, inst_id};
        default:
            break;
    }
    OC_ASSERT(false);
    return {type_id, inst_id};
}

Float LightSampler::PMF(const LightSampleContext &lsc, const Uint &index) const noexcept {
    if (env_separate_) {
        if (env_prob() == 1) {
            return 1.f;
        } else if (env_prob() == 0) {
            return PMF_(lsc, index);
        }
        Float ret = 0;
        $if(index == env_index()) {
            ret = env_prob();
        }
        $else {
            ret = (1 - env_prob()) * PMF_(lsc, index);
        };
        return ret;
    }
    return PMF_(lsc, index);
}

SampledLight LightSampler::select_light(const LightSampleContext &lsc, Float u) const noexcept {
    if (env_separate_) {
        if (env_prob() == 1) {
            return SampledLight{0, 1.f};
        } else if (env_prob() == 0) {
            return select_light_(lsc, u);
        }
        SampledLight sampled_light;
        $if(u < env_prob()) {
            sampled_light = SampledLight{env_index(), env_prob()};
        }
        $else {
            u = remapping(u, env_prob(), 1);
            sampled_light = select_light_(lsc, u);
            sampled_light.PMF *= 1 - env_prob();
        };
        return sampled_light;
    }
    return select_light_(lsc, u);
}

LightSample LightSampler::sample_wi(const SampledLight &sampled_light, const LightSampleContext &lsc,
                                    const Float2 &u, const SampledWavelengths &swl) const noexcept {
    LightSample ls{swl.dimension()};
    auto [type_id, inst_id] = extract_light_id(sampled_light.light_index);
    dispatch_light(type_id, inst_id, [&](const Light *light) {
        ls = light->sample_wi(lsc, u, swl);
    });
    ls.eval.pdf *= sampled_light.PMF;
    return ls;
}

LightSample LightSampler::sample_wi(const LightSampleContext &lsc, TSampler &sampler,
                                    const SampledWavelengths &swl) const noexcept {
    Float u_light = sampler->next_1d();
    Float2 u_surface = sampler->next_2d();
    SampledLight sampled_light = select_light(lsc, u_light);
    return sample_wi(sampled_light, lsc, u_surface, swl);
}

LightSurfacePoint LightSampler::sample_only(const LightSampleContext &lsc, TSampler &sampler) const noexcept {
    LightSurfacePoint lsp;
    SampledLight sampled_light = select_light(lsc, sampler->next_1d());
    auto [type_id, inst_id] = extract_light_id(sampled_light.light_index);
    Float2 u = sampler->next_2d();
    dispatch_light(type_id, inst_id, [&](const Light *light) {
        lsp = light->sample_only(u);
    });
    lsp.light_index = sampled_light.light_index;
    return lsp;
}

LightSample LightSampler::evaluate_point(const LightSampleContext &lsc, const LightSurfacePoint &lsp,
                                         const SampledWavelengths &swl, LightEvalMode mode) const noexcept {
    auto [type_id, inst_id] = extract_light_id(lsp.light_index);
    Float pmf = PMF(lsc, lsp.light_index);
    LightSample ls{swl.dimension()};
    dispatch_light(type_id, inst_id, [&](const Light *light) {
        ls = light->evaluate_point(lsc, lsp, swl, mode);
    });
    ls.eval.pdf *= pmf;
    return ls;
}

Float LightSampler::PDF_point(const LightSampleContext &lsc, const LightSurfacePoint &lsp,
                              const Float &pdf_wi) const noexcept {
    auto [type_id, inst_id] = extract_light_id(lsp.light_index);
    Float ret = 0.f;
    dispatch_light(type_id, inst_id, [&](const Light *light) {
        ret = light->PDF_point(lsc, lsp, pdf_wi);
    });
    return ret;
}

LightEval LightSampler::evaluate_hit_wi(const LightSampleContext &p_ref, const Interaction &it,
                                        const SampledWavelengths &swl, LightEvalMode mode) const noexcept {
    LightEval ret = LightEval{swl.dimension()};
    Uint light_idx = extract_light_index(it);
    dispatch_light(it.light_id(), [&](const Light *light) {
        if (!light->match(LightType::Area)) {
            return;
        }
        LightEvalContext p_light{it};
        p_light.PDF_pos *= light->PMF(it.prim_id);
        ret = light->evaluate_wi(p_ref, p_light, swl, mode);
    });
    Float pmf = PMF(p_ref, light_idx);
    ret.pdf *= pmf;
    return ret;
}

LightEval LightSampler::evaluate_hit_point(const LightSampleContext &p_ref, const Interaction &it,
                                           const Float &pdf_wi,
                                           const SampledWavelengths &swl,
                                           Float *light_pdf_point, LightEvalMode mode) const noexcept {
    LightEval ret = LightEval{swl.dimension()};
    Uint light_idx = extract_light_index(it);
    dispatch_light(it.light_id(), [&](const Light *light) {
        if (!light->match(LightType::Area)) {
            return;
        }
        LightEvalContext p_light{it};
        ret = light->evaluate_point(p_ref, p_light, pdf_wi, swl, mode);
        if (light_pdf_point) {
            Float prim_pmf = light->PMF(it.prim_id);
            Float light_pmf = PMF(p_ref, light_idx);
            *light_pdf_point = p_light.PDF_pos * prim_pmf * light_pmf;
        }
    });
    return ret;
}

LightEval LightSampler::evaluate_miss_wi(const LightSampleContext &p_ref, const Float3 &wi,
                                         const SampledWavelengths &swl, LightEvalMode mode) const noexcept {
    LightEvalContext p_light{p_ref.pos + wi};
    LightEval ret{swl.dimension()};
    dispatch_environment([&](const Environment *env) {
        ret = env->evaluate_wi(p_ref, p_light, swl, mode);
    });
    Float pmf = PMF(p_ref, env_index());
    ret.pdf *= pmf;
    return ret;
}

LightEval LightSampler::evaluate_miss_point(const LightSampleContext &p_ref, const Float3 &wi,
                                            const Float &pdf_wi, const SampledWavelengths &swl,
                                            Float *light_pdf_point, LightEvalMode mode) const noexcept {
    LightEvalContext p_light{p_ref.pos + wi};
    LightEval ret = env_light()->evaluate_wi(p_ref, p_light, swl, mode);
    Float light_pmf = PMF(p_ref, env_index());
    if (light_pdf_point) {
        *light_pdf_point = ret.pdf * light_pmf;
    }
    ret.pdf = pdf_wi;
    return ret;
}

void LightSampler::dispatch_environment(const std::function<void(const Environment *)> &func) const noexcept {
    auto lambda = [&](const Light *light) {
        auto env = dynamic_cast<const Environment *>(light);
        if (env) {
            func(env);
        }
    };

    if (lights_.mode() == PolymorphicMode::EInstance) {
        lights_.dispatch_instance(env_index(),lambda);
    } else {
        uint type_index = lights_.type_index(env_light());
        dispatch_light(type_index, 0, lambda);
    }
}

void LightSampler::dispatch_light(const Uint &id, const std::function<void(const Light *)> &func) const noexcept {
    lights_.dispatch(id, func);
}

void LightSampler::dispatch_light(const Uint &type_id, const Uint &inst_id,
                                  const std::function<void(const Light *)> &func) const noexcept {
    lights_.dispatch(type_id, inst_id, func);
}

}// namespace vision