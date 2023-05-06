//
// Created by Zero on 2023/5/5.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

class MixBxDFSet : public BxDFSet {
private:
    UP<BxDFSet> _b0;
    UP<BxDFSet> _b1;
    Float _scale;

public:
    [[nodiscard]] SampledSpectrum albedo() const noexcept override {
        return _b0->albedo() * _scale + _b1->albedo() * (1 - _scale);
    }
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override {
        return *(_b0->is_dispersive()) || *(_b1->is_dispersive());
    }
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