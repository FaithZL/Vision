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
    VS_MAKE_MATERIAL_EVALUATOR(LobeSet)

public:
    MixMaterial() = default;
    explicit MixMaterial(const MaterialDesc &desc)
        : Material(desc),
          mat0_(Node::create_shared<Material>(*desc.mat0)),
          mat1_(Node::create_shared<Material>(*desc.mat1)) {}
    void initialize_(const vision::NodeDesc &node_desc) noexcept override {
        VS_CAST_DESC
        mat0_->initialize_(*desc.mat0);
        mat1_->initialize_(*desc.mat1);
        frac_.set(Slot::create_slot(desc.slot("frac", 0.5f, Number)));
        init_slot_cursor(addressof(frac_), 1);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(Material, frac_, mat0_, mat1_)
    VS_MAKE_GUI_STATUS_FUNC(Material, frac_, mat0_, mat1_)
    OC_ENCODABLE_FUNC(Material, *mat0_, *mat1_)
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        return hash64(mat0_->topology_hash(), mat1_->topology_hash(), frac_.topology_hash());
    }
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        Material::render_sub_UI(widgets);
        widgets->use_tree(ocarina::format("mat0 {}", mat0_->impl_type()), [&] {
            mat0_->render_sub_UI(widgets);
        });
        widgets->use_tree(ocarina::format("mat1 {}", mat1_->impl_type()), [&] {
            mat1_->render_sub_UI(widgets);
        });
    }
    [[nodiscard]] uint64_t compute_hash() const noexcept override {
        return hash64(mat0_->hash(), mat1_->hash(), frac_.hash());
    }

    void prepare() noexcept override {
        Material::prepare();
        frac_->prepare();
        mat0_->prepare();
        mat1_->prepare();
    }

    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        WeightedLobe wb0{1, 1, mat0_->create_lobe_set(it, swl)};
        LobeSet::Lobes lobes;
        lobes.push_back(std::move(wb0));
        auto ret = make_unique<LobeSet>(std::move(lobes));;
        return ret;




        return mat0_->create_lobe_set(it, swl);
//        Float frac = frac_.evaluate(it, swl)[0];
//        return make_unique<MixLobe>(frac, mat0_->create_lobe_set(it, swl),
//                                    mat1_->create_lobe_set(it, swl));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MixMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-mix.dll")
