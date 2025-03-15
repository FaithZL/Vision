//
// Created by ling.zhu on 2025/3/15.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "math/warp.h"

namespace vision {

class MetallicMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(edge_tint)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(anisotropic)
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MicrofacetLobe)
public:
    MetallicMaterial() = default;
    explicit MetallicMaterial(const MaterialDesc &desc)
        : Material(desc) {
        INIT_SLOT(color, make_float3(1.f), Albedo);
        INIT_SLOT(edge_tint, make_float3(0.f), Albedo);
        INIT_SLOT(roughness, 0.5f, Number)->set_range(0.0001f, 1.f);
        INIT_SLOT(anisotropic, 0.f, Number)->set_range(-1,1);
        init_slot_cursor(&color_, &anisotropic_);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(Material, remapping_roughness_, alpha_threshold_)
    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it,const SampledWavelengths &swl) const noexcept override {
        return nullptr;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MetallicMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-metallic.dll")