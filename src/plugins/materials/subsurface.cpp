//
// Created by Zero on 09/12/2022.
//

#include "base/scattering/material.h"
#include "base/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class SubsurfaceMaterial : public Material {
private:
    float _sigma_scale{1.f};
    const ShaderNode *_sigma_a{};
    const ShaderNode *_sigma_s{};
    const ShaderNode *_color{};
    const ShaderNode *_ior{};
    const ShaderNode *_roughness{};
    bool _remapping_roughness{false};

public:
    explicit SubsurfaceMaterial(const MaterialDesc &desc)
        : Material(desc),
        _sigma_a{desc.scene->load_shader_node(desc.sigma_a)},
        _sigma_s{desc.scene->load_shader_node(desc.sigma_s)},
        _sigma_scale{desc.sigma_scale},
        _color(desc.scene->load_shader_node(desc.color)),
        _ior(desc.scene->load_shader_node(desc.ior)),
        _roughness(desc.scene->load_shader_node(desc.roughness)),
        _remapping_roughness(desc.remapping_roughness) {}
};

}// namespace vision