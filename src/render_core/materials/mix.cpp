//
// Created by Zero on 2023/5/5.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class MixBxDFSet : public BxDFSet {
private:
    SP<BxDFSet> _b0;
    SP<BxDFSet> _b1;
    Float _scale;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_b0->type_hash(), _b1->type_hash());
    }

public:
    MixBxDFSet(SP<BxDFSet> &&b0, SP<BxDFSet> &&b1, Float scale)
        : _b0(ocarina::move(b0)), _b1(ocarina::move(b1)), _scale(scale) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override {
        return _b0->albedo() * _scale + _b1->albedo() * (1 - _scale);
    }
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override {
        optional<Bool> v0 = _b0->is_dispersive();
        optional<Bool> v1 = _b1->is_dispersive();
        if (v0 && v1) {
            return (*v0) || (*v1);
        } else if (v0) {
            return v0;
        } else if (v1) {
            return v1;
        }
        return {};
    }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi,
                                             Uint flag) const noexcept override {
        ScatterEval eval0 = _b0->evaluate_local(wo, wi, flag);
        ScatterEval eval1 = _b1->evaluate_local(wo, wi, flag);
        ScatterEval ret{eval0.f.dimension()};
        ret.f = eval0.f * _scale + eval1.f * (1 - _scale);
        ret.pdf = eval0.pdf * _scale + eval1.pdf * (1 - _scale);
        // todo review this
        ret.flags = select(eval0.pdf > 0.f, eval0.flags, eval1.flags);
        return ret;
    }

    SampledDirection sample_wi(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        SampledDirection sd;
        Float u = sampler->next_1d();
        $if(u < _scale) {
            sd = _b0->sample_wi(wo, flag, sampler);
        }
        $else {
            sd = _b1->sample_wi(wo, flag, sampler);
        };
        return sd;
    }

    BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        BSDFSample ret{_b0->albedo().dimension()};
        SampledDirection sd = sample_wi(wo, flag, sampler);
        ret.eval = evaluate_local(wo, sd.wi, flag);
        ret.wi = sd.wi;
        ret.eval.pdf = select(sd.valid(), ret.eval.pdf * sd.pdf, 0.f);
        return ret;
    }
};

class MixMaterial : public Material {
private:
    SP<Material> _mat0{};
    SP<Material> _mat1{};
    Slot _scale{};

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<MixBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    explicit MixMaterial(const MaterialDesc &desc)
        : Material(desc),
          _mat0(scene().load<Material>(*desc.mat0)),
          _mat1(scene().load<Material>(*desc.mat1)),
          _scale(scene().create_slot(desc.slot("scale", 0.5f, Number))) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    OC_SERIALIZABLE_FUNC(Material, *_mat0, *_mat1, *_scale.node())
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_mat0->type_hash(), _mat1->type_hash(), _scale.type_hash());
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(_mat0->hash(), _mat1->hash(), _scale.hash());
    }

    void prepare() noexcept override {
        _mat0->prepare();
        _mat1->prepare();
        _scale->prepare();
    }

    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        Float scale = _scale.evaluate(it, swl)[0];
        return make_unique<MixBxDFSet>(_mat0->create_lobe_set(it, swl), _mat1->create_lobe_set(it, swl), scale);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MixMaterial)