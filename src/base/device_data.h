//
// Created by Zero on 25/10/2022.
//

#pragma once

#include "rhi/common.h"
#include "shape.h"
#include "interaction.h"

namespace vision {
using namespace ocarina;
struct DeviceData {
public:
    Device *device;
    Managed<Vertex> vertices;
    Managed<Triangle> triangles;
    Managed<Shape::Handle> instances;
    Managed<Mesh::Handle> mesh_handles;
    vector<ocarina::Mesh> meshes;
    ocarina::Accel accel;

public:
    explicit DeviceData(Device *device = nullptr)
        : device(device) {}

    void accept(const vector<Vertex> &vert, const vector<Triangle> &tri, float4x4 o2w, uint mat_id, uint light_id);
    void reset_device_buffer();
    void build_meshes();
    void build_accel();
    void upload() const;
    [[nodiscard]]
    [[nodiscard]] LightEvalContext compute_light_eval_context(const Uint &inst_id,
                                                              const Uint &prim_id,
                                                              const Float2 &bary) const noexcept;
    [[nodiscard]] array<Var<Vertex>, 3> get_vertices(const Var<Triangle> &tri, const Uint &offset) const noexcept;
    [[nodiscard]] SurfaceInteraction compute_surface_interaction(const OCHit &hit, bool is_complete = true) const noexcept;
};
}// namespace vision