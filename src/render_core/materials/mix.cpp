//
// Created by Zero on 2023/5/5.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class MixMaterial : public Material {
private:
    VS_MAKE_SLOT(frac)
    SP<Material> mat0_{};
    SP<Material> mat1_{};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MixLobe)

public:
    explicit MixMaterial(const MaterialDesc &desc)
        : Material(desc),
          mat0_(Node::create_shared<Material>(*desc.mat0)),
          mat1_(Node::create_shared<Material>(*desc.mat1)) {
        frac_.set(Slot::create_slot(desc.slot("frac", 0.5f, Number)));
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    OC_ENCODABLE_FUNC(Material, *frac_.node(), *mat0_, *mat1_)
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(mat0_->type_hash(), mat1_->type_hash(), frac_.type_hash());
    }

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        Material::render_sub_UI(widgets);
        widgets->use_tree("material 0", [&] {
            mat0_->render_sub_UI(widgets);
        });
        widgets->use_tree("material 1", [&] {
            mat1_->render_sub_UI(widgets);
        });
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(mat0_->hash(), mat1_->hash(), frac_.hash());
    }

    void prepare() noexcept override {
        frac_->prepare();
        mat0_->prepare();
        mat1_->prepare();
    }

    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        Float frac = frac_.evaluate(it, swl)[0];
        return make_unique<MixLobe>(frac, mat0_->create_lobe_set(it, swl),
                                    mat1_->create_lobe_set(it, swl));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MixMaterial)