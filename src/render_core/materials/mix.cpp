//
// Created by Zero on 2023/5/5.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

class MixBSDF : public BSDF {
private:
    UP<BSDF> _bsdf0{};
    UP<BSDF> _bsdf1{};
    Float _scale{};

public:
    MixBSDF(UP<BSDF> &&b0, UP<BSDF> &&b1, Float scale,
            const Interaction &it,const SampledWavelengths &swl)
        : BSDF(it, swl), _bsdf0(move(b0)), _bsdf1(move(b1)), _scale(scale) {}
};

class MixMaterial : public Material {
private:
    Material *_mat0{};
    Material *_mat1{};
    Slot _scale{};

public:
    explicit MixMaterial(const MaterialDesc &desc)
        : Material(desc), _mat0(desc.scene->load<Material>(*desc.mat0)),
          _mat1(desc.scene->load<Material>(*desc.mat1)),
          _scale(_scene->create_slot(desc.slot("scale", make_float3(0.5f), Number))) {}

    OC_SERIALIZABLE_FUNC((*_mat0), (*_mat1), (*_scale.node()))
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_mat0->type_hash(), _mat1->type_hash(), _scale.type_hash());
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(_mat0->hash(), _mat1->hash(), _scale.hash());
    }
};

}// namespace vision