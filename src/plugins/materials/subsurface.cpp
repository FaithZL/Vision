//
// Created by Zero on 09/12/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class SubsurfaceMaterial : public Material {
private:
    const ShaderNode *_sigma_scale{};
    const ShaderNode *_sigma_a{};
    const ShaderNode *_sigma_s{};
    const ShaderNode *_color{};
    const ShaderNode *_ior{};
    const ShaderNode *_roughness{};
    bool _remapping_roughness{false};

public:
    explicit SubsurfaceMaterial(const MaterialDesc &desc)
        : Material(desc),
          _sigma_a{_scene->load_shader_node(desc.attr("sigma_a", make_float3(.0011f, .0024f, .014f), Unbound))},
          _sigma_s{_scene->load_shader_node(desc.attr("sigma_s", make_float3(2.55f, 3.21f, 3.77f), Unbound))},
          _sigma_scale{_scene->load_shader_node(desc.attr("sigma_scale", 1.f, Number))},
          _color(_scene->load_shader_node(desc.attr("color", make_float3(1.f), Albedo))),
          _ior(_scene->load_shader_node(desc.attr("ior", make_float3(1.5f)))),
          _roughness(_scene->load_shader_node(desc.attr("roughness", make_float2(0.1f)))),
          _remapping_roughness(desc["remapping_roughness"].as_bool(false)) {}
};

}// namespace vision