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

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MicrofacetLobe)
public:
    MetallicMaterial() = default;
    explicit MetallicMaterial(const MaterialDesc &desc)
        : Material(desc) {

    }
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it,
                                           const SampledWavelengths &swl) const noexcept override {
        return nullptr;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MetallicMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-metallic.dll")