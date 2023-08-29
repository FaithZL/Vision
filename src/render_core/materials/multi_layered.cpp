//
// Created by Zero on 2023/8/28.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class MultiLayeredMaterial : public Material {
private:
    static constexpr float float_min = std::numeric_limits<float>::min();

private:
    Serial<float> _factor{float_min};
    Slot _thickness;
    SP<Material> _bottom{};
    SP<Material> _top{};

public:
    explicit MultiLayeredMaterial(const MaterialDesc &desc)
        : Material(desc),
          _thickness(scene().create_slot(desc.slot("_thickness", 1.f, Number))),
          _bottom(scene().load<Material>(*desc.mat0)),
          _top(scene().load<Material>(*desc.mat1)) {}

    void prepare() noexcept override {
        _bottom->prepare();
        _top->prepare();
        _thickness->prepare();
    }
};

}// namespace vision