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
    DCUP<BxDFSet> b0_;
    DCUP<BxDFSet> b1_;
    Float scale_;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(b0_->type_hash(), b1_->type_hash());
    }

public:
    VS_MAKE_BxDFSet_ASSIGNMENT(MixBxDFSet)
        MixBxDFSet(UP<BxDFSet> &&b0, UP<BxDFSet> &&b1, Float scale)
        : b0_(ocarina::move(b0)), b1_(ocarina::move(b1)), scale_(scale) {}

    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override {
        return b0_->albedo(wo) * scale_ + b1_->albedo(wo) * (1 - scale_);
    }
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override {
        optional<Bool> v0 = b0_->is_dispersive();
        optional<Bool> v1 = b1_->is_dispersive();
        if (v0 && v1) {
            return (*v0) || (*v1);
        } else if (v0) {
            return v0;
        } else if (v1) {
            return v1;
        }
        return {};
    }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, MaterialEvalMode mode,
                                             Uint flag) const noexcept override {
        ScatterEval eval0 = b0_->evaluate_local(wo, wi, mode, flag);
        ScatterEval eval1 = b1_->evaluate_local(wo, wi, mode, flag);
        ScatterEval ret{eval0.f.dimension()};
        ret.f = eval0.f * scale_ + eval1.f * (1 - scale_);
        ret.pdf = eval0.pdf * scale_ + eval1.pdf * (1 - scale_);
        // todo review this
        ret.flags = select(eval0.pdf > 0.f, eval0.flags, eval1.flags);
        return ret;
    }

    SampledDirection sample_wi(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        SampledDirection sd;
        Float u = sampler->next_1d();
        $if(u < scale_) {
            sd = b0_->sample_wi(wo, flag, sampler);
        }
        $else {
            sd = b1_->sample_wi(wo, flag, sampler);
        };
        return sd;
    }

    BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        BSDFSample ret{b0_->albedo(wo).dimension()};
        SampledDirection sd = sample_wi(wo, flag, sampler);
        ret.eval = evaluate_local(wo, sd.wi, MaterialEvalMode::All, flag);
        ret.wi = sd.wi;
        ret.eval.pdf = select(sd.valid(), ret.eval.pdf * sd.pdf, 0.f);
        return ret;
    }
};

class MixMaterial : public Material {
private:
    SP<Material> mat0_{};
    SP<Material> mat1_{};
    VS_MAKE_SLOT(scale)

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<MixBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    explicit MixMaterial(const MaterialDesc &desc)
        : Material(desc),
          mat0_(scene().load<Material>(*desc.mat0)),
          mat1_(scene().load<Material>(*desc.mat1)) {
        _scale.set(scene().create_slot(desc.slot("scale", 0.5f, Number)));
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    OC_SERIALIZABLE_FUNC(Material, *mat0_, *mat1_, *_scale.node())
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(mat0_->type_hash(), mat1_->type_hash(), _scale.type_hash());
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(mat0_->hash(), mat1_->hash(), _scale.hash());
    }

    void prepare() noexcept override {
        mat0_->prepare();
        mat1_->prepare();
        _scale->prepare();
    }

    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        Float scale = _scale.evaluate(it, swl)[0];
        return make_unique<MixBxDFSet>(mat0_->create_lobe_set(it, swl), mat1_->create_lobe_set(it, swl), scale);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MixMaterial)