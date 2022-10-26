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

    void accept(const vector<Vertex> &vert, const vector<Triangle> &tri, float4x4 o2w);
    void reset_device_buffer();
    void build_meshes();
    void build_accel();
    void upload() const;
    [[nodiscard]] array<Var<Vertex>, 3> get_vertices(const Var<Triangle> &tri, const Uint &offset) const noexcept;
    [[nodiscard]] SurfaceInteraction<D> compute_surface_interaction(const OCHit &hit) const noexcept;
};
}// namespace vision