//
// Created by Zero on 2025/1/1.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class AddBxDFSet : public BxDFSet {
private:
    DCUP<BxDFSet> b0_;
    DCUP<BxDFSet> b1_;

public:
    VS_MAKE_BxDFSet_ASSIGNMENT(AddBxDFSet)
        AddBxDFSet(UP<BxDFSet> &&b0, UP<BxDFSet> &&b1)
        : b0_(ocarina::move(b0)), b1_(ocarina::move(b1)) {
    }
    [[nodiscard]] Uint flag() const noexcept override { return b0_->flag() | b1_->flag(); }
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override {
        return b0_->albedo(cos_theta) + b1_->albedo(cos_theta);
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
    [[nodiscard]] const SampledWavelengths *swl() const override {
        return b0_->swl();
    }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override {
        ScatterEval eval0 = b0_->evaluate_local(wo, wi, mode, flag);
        ScatterEval eval1 = b1_->evaluate_local(wo, wi, mode, flag);
        ScatterEval ret{eval0.f.dimension(), eval0.pdfs.size()};
        ret.f = eval0.f + eval1.f;
        ret.pdfs = eval0.pdf() + eval1.pdf();
        // todo review this
        ret.flags = select(eval0.pdf() > 0.f, eval0.flags, eval1.flags);
        return ret;
    }

    SampledDirection sample_wi(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        SampledDirection sd;
        Float u = sampler->next_1d();
        $if(u > 0.5f) {
            sd = b0_->sample_wi(wo, flag, sampler);
        }
        $else {
            sd = b1_->sample_wi(wo, flag, sampler);
        };
        return sd;
    }

    BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        BSDFSample ret{*b0_->swl()};
        SampledDirection sd = sample_wi(wo, flag, sampler);
        ret.eval = evaluate_local(wo, sd.wi, MaterialEvalMode::All, flag);
        ret.wi = sd.wi;
        ret.eval.pdfs = select(sd.valid(), ret.eval.pdf() * sd.pdf, 0.f);
        return ret;
    }
};

class AddMaterial : public Material {
private:
    SP<Material> mat0_{};
    SP<Material> mat1_{};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(AddBxDFSet)

public:
    AddMaterial() = default;
    VS_MAKE_PLUGIN_NAME_FUNC
    explicit AddMaterial(const MaterialDesc &desc)
        : Material(desc),
          mat0_(Node::create_shared<Material>(*desc.mat0)),
          mat1_(Node::create_shared<Material>(*desc.mat1)) {
    }
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        return make_unique<AddBxDFSet>(mat0_->create_lobe_set(it, swl), mat1_->create_lobe_set(it, swl));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, AddMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-add.dll")