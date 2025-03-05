//
// Created by Zero on 06/11/2022.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class BlackBodyMaterial : public Material {
protected:
    VS_MAKE_MATERIAL_EVALUATOR(BlackBodyLobe)

public:
    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        return make_unique<BlackBodyLobe>(swl);
    }
    [[nodiscard]] bool enable_delta() const noexcept override { return false; }
    explicit BlackBodyMaterial(const MaterialDesc &desc)
        : Material(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BlackBodyMaterial)