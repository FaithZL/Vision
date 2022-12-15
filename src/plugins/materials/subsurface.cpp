//
// Created by Zero on 09/12/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class SubsurfaceMaterial : public Material {
private:
    float _sigma_scale{1.f};
    const Texture *_sigma_a{};
    const Texture *_sigma_s{};
    const Texture *_color{};
    const Texture *_ior{};
    const Texture *_roughness{};
    bool _remapping_roughness{false};

public:
    explicit SubsurfaceMaterial(const MaterialDesc &desc)
        : Material(desc),
        _sigma_a{desc.scene->load<Texture>(desc.sigma_a)},
        _sigma_s{desc.scene->load<Texture>(desc.sigma_s)},
        _sigma_scale{desc.sigma_scale},
        _color(desc.scene->load<Texture>(desc.color)),
        _ior(desc.scene->load<Texture>(desc.ior)),
        _roughness(desc.scene->load<Texture>(desc.roughness)),
        _remapping_roughness(desc.remapping_roughness) {}
};

}// namespace vision