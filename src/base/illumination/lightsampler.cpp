//
// Created by Zero on 22/10/2022.
//

#include "lightsampler.h"
#include "base/mgr/pipeline.h"

namespace vision {

LightSampler::LightSampler(const LightSamplerDesc &desc)
    : Node(desc),
      _env_separate(desc["env_separate"].as_bool(false)),
      _env_prob(ocarina::clamp(desc["env_prob"].as_float(0.5f), 0.01f, 0.99f)) {
    for (const LightDesc &light_desc : desc.light_descs) {
        SP<Light> light = scene().load<Light>(light_desc);
        if (light->match(LightType::Area)) {
            SP<IAreaLight> emission = std::dynamic_pointer_cast<IAreaLight>(light);
            emission->instance()->set_emission(emission);
        }
        if (light->match(LightType::Infinite)) {
            _env_light = std::dynamic_pointer_cast<Environment>(light);
        }
        add_light(light);
    }
}

Uint LightSampler::correct_index(Uint index) const noexcept {
    if (env_light() && _env_separate) {
        return ocarina::select(index < _env_light->index(), index, index + 1u);
    }
    return index;
}

void LightSampler::tidy_up() noexcept {
    std::sort(_lights.begin(), _lights.end(), [&](SP<Light> a, SP<Light> b) {
        return _lights.type_index(a.get()) < _lights.type_index(b.get());
    });
    for_each([&](SP<Light> light, uint index) noexcept {
        light->set_index(index);
    });
}

void LightSampler::prepare() noexcept {
    for_each([&](SP<Light> light, uint index) noexcept {
        light->prepare();
    });
    auto rp = pipeline();
    _lights.prepare(rp->resource_array(), rp->device());
}

Uint LightSampler::combine_to_light_index(const Uint &type_id, const Uint &inst_id) const noexcept {
    vector<uint> func;
    Uint ret = 0u;
    switch (_lights.mode()) {
        case ocarina::EInstance: {
            ret = inst_id;
            break;
        }
        case ocarina::EType: {
            func.reserve(_lights.type_num());
            for (int i = 0; i < _lights.type_num(); ++i) {
                func.push_back(static_cast<uint>(_lights.instance_num(i)));
            }
            Array<uint> arr{func};
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
    vector<uint> func;
    func.reserve(_lights.type_num());
    for (int i = 0; i < _lights.type_num(); ++i) {
        func.push_back(static_cast<uint>(_lights.instance_num(i)));
    }

    Uint accum = 0u;
    for (uint i = 0; i < func.size(); ++i) {
        type_id = select(index >= accum, i, type_id);
        inst_id = select(index >= accum, index - accum, inst_id);
        accum += func[i];
    }
    switch (_lights.mode()) {
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

LightEval LightSampler::evaluate_hit_wi(const LightSampleContext &p_ref, const Interaction &it,
                                        const SampledWavelengths &swl) const noexcept {
    LightEval ret = LightEval{swl.dimension()};
    dispatch_light(it.light_id(), [&](const Light *light) {
        if (!light->match(LightType::Area)) {
            return;
        }
        LightEvalContext p_light{it};
        p_light.PDF_pos *= light->PMF(it.prim_id);
        ret = light->evaluate_wi(p_ref, p_light, swl);
        Float pmf = PMF(p_ref, combine_to_light_index(it.light_type_id(), it.light_inst_id()));
        ret.pdf *= pmf;
    });
    return ret;
}

LightEval LightSampler::evaluate_hit_point(const LightSampleContext &p_ref, const Interaction &it,
                                           const SampledWavelengths &swl) const noexcept {
    LightEval ret = LightEval{swl.dimension()};
    dispatch_light(it.light_id(), [&](const Light *light) {
        if (!light->match(LightType::Area)) {
            return;
        }
        LightEvalContext p_light{it};
        p_light.PDF_pos *= light->PMF(it.prim_id);
        ret = light->evaluate_point(p_ref, p_light, swl);
        Float pmf = PMF(p_ref, combine_to_light_index(it.light_type_id(), it.light_inst_id()));
        ret.pdf *= pmf;
    });
    return ret;
}

Float LightSampler::PMF(const LightSampleContext &lsc, const Uint &index) const noexcept {
    if (_env_separate) {
        if (env_prob() == 1) {
            return 1.f;
        } else if (env_prob() == 0) {
            return _PMF(lsc, index);
        }
        Float ret = 0;
        $if(index == _env_light->index()) {
            ret = env_prob();
        }
        $else {
            ret = (1 - env_prob()) * _PMF(lsc, index);
        };
        return ret;
    }
    return _PMF(lsc, index);
}

SampledLight LightSampler::select_light(const LightSampleContext &lsc, Float u) const noexcept {
    if (_env_separate) {
        if (env_prob() == 1) {
            return SampledLight{0, 1.f};
        } else if (env_prob() == 0) {
            return _select_light(lsc, u);
        }
        SampledLight sampled_light;
        $if(u < env_prob()) {
            sampled_light = SampledLight{_env_light->index(), env_prob()};
        }
        $else {
            u = remapping(u, env_prob(), 1);
            sampled_light = _select_light(lsc, u);
            sampled_light.PMF *= 1 - env_prob();
        };
        return sampled_light;
    }
    return _select_light(lsc, u);
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

LightSample LightSampler::sample_wi(const LightSampleContext &lsc, Sampler *sampler,
                                    const SampledWavelengths &swl) const noexcept {
    Float u_light = sampler->next_1d();
    Float2 u_surface = sampler->next_2d();
    SampledLight sampled_light = select_light(lsc, u_light);
    return sample_wi(sampled_light, lsc, u_surface, swl);
}

LightSample LightSampler::sample_point(const SampledLight &sampled_light, const LightSampleContext &lsc,
                                       const Float2 &u, const SampledWavelengths &swl) const noexcept {
    LightSample ls{swl.dimension()};
    auto [type_id, inst_id] = extract_light_id(sampled_light.light_index);
    dispatch_light(type_id, inst_id, [&](const Light *light) {
        ls = light->sample_point(lsc, u, swl);
    });
    ls.eval.pdf *= sampled_light.PMF;
    return ls;
}

LightSample LightSampler::sample_point(const LightSampleContext &lsc, Sampler *sampler,
                                       const SampledWavelengths &swl) const noexcept {
    Float u_light = sampler->next_1d();
    Float2 u_surface = sampler->next_2d();
    SampledLight sampled_light = select_light(lsc, u_light);
    return sample_point(sampled_light, lsc, u_surface, swl);
}

LightEval LightSampler::evaluate_miss(const LightSampleContext &p_ref, Float3 wi,
                                      const SampledWavelengths &swl) const noexcept {
    LightEvalContext p_light{p_ref.pos + wi};
    LightEval ret = env_light()->evaluate_wi(p_ref, p_light, swl);
    Float pmf = PMF(p_ref, _env_light->index());
    ret.pdf *= pmf;
    return ret;
}

void LightSampler::dispatch_light(const Uint &id, const std::function<void(const Light *)> &func) const noexcept {
    _lights.dispatch(id, func);
}

void LightSampler::dispatch_light(const Uint &type_id, const Uint &inst_id,
                                  const std::function<void(const Light *)> &func) const noexcept {
    _lights.dispatch(type_id, inst_id, func);
}

}// namespace vision