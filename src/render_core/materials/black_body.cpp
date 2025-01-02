//
// Created by Zero on 06/11/2022.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class BlackBodyMaterial : public Material {
protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<BlackBodyBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        return make_unique<BlackBodyBxDFSet>(swl);
    }
    [[nodiscard]] bool enable_delta() const noexcept override { return false; }
    explicit BlackBodyMaterial(const MaterialDesc &desc)
        : Material(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BlackBodyMaterial)