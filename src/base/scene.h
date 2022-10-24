//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "descriptions/scene_desc.h"
#include "base/node.h"
#include "base/sensor.h"
#include "base/sampler.h"
#include "base/shape.h"
#include "base/integrator.h"
#include "base/lightsampler.h"
#include "base/material.h"

namespace vision {

using namespace ocarina;

class Context;

class Scene {
private:
    vision::Context *_context{nullptr};
    vector<Node::Wrapper> _all_nodes;
    Box3f _aabb;
    Camera *_camera{nullptr};
    Sampler *_sampler{nullptr};
    Integrator *_integrator{nullptr};
    LightSampler *_light_sampler{nullptr};
    vector<Shape *> _shapes;
    vector<Material *> _materials;
    vector<Mesh::Handle> _meshes;
    friend class RenderPipeline;

public:
    explicit Scene(vision::Context *ctx);
    void init(const SceneDesc &scene_desc);
    void prepare(RenderPipeline *rp) noexcept;
    [[nodiscard]] auto camera() const noexcept { return _camera; }
    [[nodiscard]] auto camera() noexcept { return _camera; }
    [[nodiscard]] auto film() noexcept { return camera()->film(); }
    [[nodiscard]] auto film() const noexcept { return camera()->film(); }
    [[nodiscard]] auto integrator() const noexcept { return _integrator; }
    [[nodiscard]] auto integrator() noexcept { return _integrator; }
    [[nodiscard]] Node *load_node(const NodeDesc &desc);
    template<typename T, typename desc_ty>
    [[nodiscard]] T *load(const desc_ty &desc) noexcept {
        auto ret = dynamic_cast<T *>(load_node(desc));
        OC_ERROR_IF(ret == nullptr, "error node load ", desc.name);
        return ret;
    }
    void load_shapes(const vector<ShapeDesc> &descs) noexcept;
    void load_materials(const vector<MaterialDesc> &material_descs) noexcept;
    Light *load_light(const LightDesc &desc) noexcept;
    void load_lights(const vector<LightDesc> &descs) noexcept;
};

}// namespace vision