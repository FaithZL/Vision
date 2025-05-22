//
// Created by Zero on 09/09/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class MatteMaterial : public Material {
private:
    VS_MAKE_SLOT(color);
    VS_MAKE_SLOT(sigma);

protected:
    VS_MAKE_MATERIAL_EVALUATOR(DiffuseLobe)

public:
    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it,
                                           const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = color_.eval_albedo_spectrum(it, swl).sample;
        if (sigma_) {
            Float sigma = sigma_.evaluate(it, swl)->as_scalar();
            return make_unique<DiffuseLobe>(kr, sigma, swl);
        }
        return make_unique<DiffuseLobe>(kr, swl);
    }
    [[nodiscard]] bool enable_delta() const noexcept override { return false; }
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        Material::render_UI(widgets);
        return true;
    }
    MatteMaterial() = default;
    explicit MatteMaterial(const MaterialDesc &desc)
        : Material(desc) {}

    void initialize_slots(const vision::Material::Desc &desc) noexcept override {
        VS_INIT_SLOT(color, make_float3(0.5f), Albedo);
        if (desc.has_attr("sigma")) {
            VS_INIT_SLOT(sigma, 0.5f, Number).set_range(0.f, 1.f);
        }
        init_slot_cursor(&color_, 2);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MatteMaterial)