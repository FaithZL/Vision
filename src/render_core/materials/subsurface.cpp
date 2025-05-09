//
// Created by Zero on 09/12/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class SubsurfaceMaterial : public Material {
private:
    float sigma_scale_{};
    VS_MAKE_SLOT(sigma_a)
    VS_MAKE_SLOT(sigma_s)
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(ior)
    VS_MAKE_SLOT(roughness)
    bool remapping_roughness_{false};

public:
    explicit SubsurfaceMaterial(const MaterialDesc &desc)
        : Material(desc),
          sigma_a_{InputSlot::create_slot(desc.slot("sigma_a", make_float3(.0011f, .0024f, .014f), Unbound))},
          sigma_s_{InputSlot::create_slot(desc.slot("sigma_s", make_float3(2.55f, 3.21f, 3.77f), Unbound))},
          sigma_scale_{desc["sigma_scale"].as_float(1.f)},
          color_(InputSlot::create_slot(desc.slot("color", make_float3(1.f), Albedo))),
          ior_(InputSlot::create_slot(desc.slot("ior", make_float3(1.5f)))),
          roughness_(InputSlot::create_slot(desc.slot("roughness", make_float2(0.1f)))),
          remapping_roughness_(desc["remapping_roughness"].as_bool(false)) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision