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
    UP<BxDFSet> _b0;
    UP<BxDFSet> _b1;
    Float _scale;

public:
    MixBxDFSet(UP<BxDFSet> &&b0, UP<BxDFSet> &&b1, Float scale)
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
    Material *_mat0{};
    Material *_mat1{};
    Slot _scale{};

public:
    explicit MixMaterial(const MaterialDesc &desc)
        : Material(desc),
          _mat0(desc.scene->load<Material>(*desc.mat0)),
          _mat1(desc.scene->load<Material>(*desc.mat1)),
          _scale(_scene->create_slot(desc.slot("scale", 0.5f, Number))) {}

    OC_SERIALIZABLE_FUNC(*_mat0, *_mat1, *_scale.node())
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

    [[nodiscard]] BSDF compute_BSDF(const Interaction &it, const SampledWavelengths &swl) const noexcept override {
        BSDF b0 = _mat0->compute_BSDF(it, swl);
        BSDF b1 = _mat1->compute_BSDF(it, swl);
        Float scale = _scale.evaluate(it, swl)[0];
        return BSDF(it, make_unique<MixBxDFSet>(ocarina::move(b0.bxdf_set), ocarina::move(b1.bxdf_set), scale));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MixMaterial)