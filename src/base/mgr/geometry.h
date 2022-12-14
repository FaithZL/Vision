//
// Created by Zero on 25/10/2022.
//

#pragma once

#include "rhi/common.h"
#include "base/shape.h"
#include "base/scattering/interaction.h"

namespace vision {
using namespace ocarina;
class Scene;
struct Geometry {
public:
    Device *device;
    Managed<Vertex> vertices;
    Managed<Triangle> triangles;
    Managed<Shape::Handle> instances;
    Managed<Mesh::Handle> mesh_handles;
    vector<ocarina::Mesh> meshes;
    ocarina::Accel accel;

private:
    [[nodiscard]] Interaction compute_surface_interaction(const OCHit &hit, bool is_complete) const noexcept;

public:
    explicit Geometry(Device *device = nullptr)
        : device(device) {}

    void accept(const vector<Vertex> &vert, const vector<Triangle> &tri, Shape::Handle handle);
    void reset_device_buffer();
    void build_meshes();
    void build_accel();
    void upload() const;

    // for dsl
    [[nodiscard]] OCHit trace_closest(const OCRay &ray) const noexcept;
    [[nodiscard]] Bool trace_any(const OCRay &ray) const noexcept;
    [[nodiscard]] Bool occluded(const Interaction &it, const Float3 &pos, RayState *rs = nullptr) const noexcept;
    [[nodiscard]] Float3 Tr(Scene *scene, const RayState &ray_state) const noexcept;
    [[nodiscard]] LightEvalContext compute_light_eval_context(const Uint &inst_id,
                                                              const Uint &prim_id,
                                                              const Float2 &bary) const noexcept;
    [[nodiscard]] array<Var<Vertex>, 3> get_vertices(const Var<Triangle> &tri, const Uint &offset) const noexcept;
    [[nodiscard]] Interaction compute_surface_interaction(const OCHit &hit, OCRay &ray) const noexcept {
        auto ret = compute_surface_interaction(hit, true);
        ret.wo = normalize(-ray->direction());
        ray.dir_max.w = length(ret.pos - ray->origin()) / length(ray->direction());
        return ret;
    }
};
}// namespace vision