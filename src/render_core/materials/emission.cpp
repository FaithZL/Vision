//
// Created by ling.zhu on 2025/6/25.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class EmissionLobe : public Lobe {

};

class EmissionMaterial : public Material {
private:
    VS_MAKE_SLOT(color);
    VS_MAKE_SLOT(strength);

public:
    EmissionMaterial() = default;
    explicit EmissionMaterial(const MaterialDesc &desc)
        : Material(desc) {}

    void initialize_slots(const vision::Material::Desc &desc) noexcept override {
        Material::initialize_slots(desc);
        VS_INIT_SLOT(color, make_float3(0.5f), Albedo);
        if (desc.has_attr("strength")) {
            VS_INIT_SLOT(strength, 0.5f, Number).set_range(0.f, 1.f);
        }
        init_slot_cursor(&color_, 2);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision