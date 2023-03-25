//
// Created by Zero on 09/12/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class SubsurfaceMaterial : public Material {
private:
    float _sigma_scale{};
    TSlot<3> _sigma_a{};
    TSlot<3> _sigma_s{};
    TSlot<3> _color{};
    TSlot<1> _ior{};
    TSlot<2> _roughness{};
    bool _remapping_roughness{false};

public:
    explicit SubsurfaceMaterial(const MaterialDesc &desc)
        : Material(desc),
          _sigma_a{_scene->create_tslot(desc.tslot<3>("sigma_a", make_float3(.0011f, .0024f, .014f), Unbound))},
          _sigma_s{_scene->create_tslot(desc.tslot<3>("sigma_s", make_float3(2.55f, 3.21f, 3.77f), Unbound))},
          _sigma_scale{desc["sigma_scale"].as_float(1.f)},
          _color(_scene->create_tslot(desc.tslot<3>("color", make_float3(1.f), Albedo))),
          _ior(_scene->create_tslot(desc.tslot<1>("ior", make_float3(1.5f)))),
          _roughness(_scene->create_tslot(desc.tslot<2>("roughness", make_float2(0.1f)))),
          _remapping_roughness(desc["remapping_roughness"].as_bool(false)) {}
};

}// namespace vision