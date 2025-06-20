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
    HotfixSlot<SP<Material>> mat0_{};
    HotfixSlot<SP<Material>> mat1_{};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(LobeSet)

public:
    MixMaterial() = default;
    explicit MixMaterial(const MaterialDesc &desc)
        : Material(desc),
          mat0_(Node::create_shared<Material>(*desc.mat0)),
          mat1_(Node::create_shared<Material>(*desc.mat1)) {
    }

    void initialize_slots(const vision::Material::Desc &desc) noexcept override {
        mat0_->set_graph(shared_graph());
        mat1_->set_graph(shared_graph());
        mat0_->initialize_slots(*desc.mat0);
        mat1_->initialize_slots(*desc.mat1);
        frac_.set(ShaderNodeSlot::create_slot(desc.slot("frac", 0.5f, Number)));
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

    [[nodiscard]] UP<Lobe> create_lobe_set(const Interaction &it, const SampledWavelengths &swl) const noexcept override {
        Float frac = frac_.evaluate(it, swl).array[0];
        auto ret = LobeSet::create_mix(frac, mat0_->create_lobe_set(it, swl),
                                       mat1_->create_lobe_set(it, swl));
        ret->set_shading_frame(it.shading);
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MixMaterial)