//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "descriptions/scene_desc.h"
#include "base/node.h"
#include "base/sensor.h"
#include "base/sampler.h"
#include "base/shape.h"
#include "base/lightsampler.h"
#include "base/material.h"

namespace vision {

using namespace ocarina;

class Context;

class Scene {
private:
    vision::Context *_context{nullptr};
    vector<Node::Handle> _all_nodes;
    Camera *_camera{nullptr};
    Sampler *_sampler{nullptr};
    LightSampler *_light_sampler{nullptr};
    vector<Shape *> _shapes;
    vector<Material *> _materials;

public:
    explicit Scene(vision::Context *ctx);
    void init(const SceneDesc& scene_desc);
    void prepare(RenderPipeline *rp) noexcept;
    [[nodiscard]] Node *load_node(const NodeDesc *desc);
    template<typename T, typename desc_ty>
    [[nodiscard]] T *load(const desc_ty *desc) noexcept {
        auto ret = dynamic_cast<T *>(load_node(desc));
        OC_ERROR_IF(ret == nullptr, "error node load ", desc->name);
        return ret;
    }
    void load_shapes(const vector<ShapeDesc> &descs) noexcept {
        for (const ShapeDesc &desc : descs) {
            _shapes.push_back(load<Shape>(&desc));
        }
    }
    void load_materials(const vector<MaterialDesc> &material_descs) noexcept {
        for (const MaterialDesc &desc : material_descs) {
            desc.scene = this;
            _materials.push_back(load<Material>(&desc));
        }
    }
};

}// namespace vision