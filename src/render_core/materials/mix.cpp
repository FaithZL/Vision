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
    Float frac_;
    DCUP<BxDFSet> b0_;
    DCUP<BxDFSet> b1_;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(b0_->type_hash(), b1_->type_hash());
    }

public:
    VS_MAKE_BxDFSet_ASSIGNMENT(MixBxDFSet)
        MixBxDFSet(UP<BxDFSet> &&b0, UP<BxDFSet> &&b1, Float frac)
        : b0_(ocarina::move(b0)), b1_(ocarina::move(b1)), frac_(frac) {
    }
    [[nodiscard]] Uint flag() const noexcept override { return b0_->flag() | b1_->flag(); }
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override {
        return b0_->albedo(cos_theta) * (1 - frac_) + b1_->albedo(cos_theta) * frac_;
    }
    [[nodiscard]] const SampledWavelengths *swl() const override {
        return b0_->swl();
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
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag,TransportMode tm) const noexcept override {
        ScatterEval eval0 = b0_->evaluate_local(wo, wi, mode, flag);
        ScatterEval eval1 = b1_->evaluate_local(wo, wi, mode, flag);
        ScatterEval ret{eval0.f.dimension(), eval0.pdfs.size()};
        ret.f = eval0.f * (1 - frac_) + eval1.f * frac_;
        ret.pdfs = eval0.pdf() * (1 - frac_) + eval1.pdf() * frac_;
        // todo review this
        ret.flags = select(eval0.pdf() > 0.f, eval0.flags, eval1.flags);
        return ret;
    }

    SampledDirection sample_wi(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        SampledDirection sd;
        Float u = sampler->next_1d();
        $if(u > frac_) {
            sd = b0_->sample_wi(wo, flag, sampler);
        }
        $else {
            sd = b1_->sample_wi(wo, flag, sampler);
        };
        return sd;
    }

    BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                            TransportMode tm) const noexcept override {
        BSDFSample ret{*b0_->swl()};
        SampledDirection sd = sample_wi(wo, flag, sampler);
        ret.eval = evaluate_local(wo, sd.wi, MaterialEvalMode::All, flag ,tm);
        ret.wi = sd.wi;
        ret.eval.pdfs *= sd.factor();
        return ret;
    }
};

class MixMaterial : public Material {
private:
    VS_MAKE_SLOT(frac)
    SP<Material> mat0_{};
    SP<Material> mat1_{};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MixBxDFSet)

public:
    explicit MixMaterial(const MaterialDesc &desc)
        : Material(desc),
          mat0_(Node::create_shared<Material>(*desc.mat0)),
          mat1_(Node::create_shared<Material>(*desc.mat1)) {
        frac_.set(Slot::create_slot(desc.slot("frac", 0.5f, Number)));
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    OC_ENCODABLE_FUNC(Material, *mat0_, *mat1_, *frac_.node())
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(mat0_->type_hash(), mat1_->type_hash(), frac_.type_hash());
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(mat0_->hash(), mat1_->hash(), frac_.hash());
    }

    void prepare() noexcept override {
        frac_->prepare();
        mat0_->prepare();
        mat1_->prepare();
    }

    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        Float frac = frac_.evaluate(it, swl)[0];
        return make_unique<MixBxDFSet>(mat0_->create_lobe_set(it, swl), mat1_->create_lobe_set(it, swl), frac);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MixMaterial)