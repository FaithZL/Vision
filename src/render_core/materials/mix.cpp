//
// Created by Zero on 2023/5/5.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

class MixMaterial : public Material {
private:
    Material *_mat0{};
    Material *_mat1{};

public:
    explicit MixMaterial(const MaterialDesc &desc)
        : Material(desc), _mat0(desc.scene->load<Material>(*desc.mat0)),
          _mat1(desc.scene->load<Material>(*desc.mat1)) {}


};

}// namespace vision