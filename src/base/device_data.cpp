//
// Created by Zero on 25/10/2022.
//

#include "device_data.h"
#include "math/transform.h"

namespace vision {

void DeviceData::accept(const vector<Vertex> &vert,
                        const vector<Triangle> &tri, float4x4 o2w) {

    Mesh::Handle mesh_handle{.vertex_offset = (uint)vertices.host().size(),
                             .triangle_offset = (uint)triangles.host().size()};

    vertices.append(vert);
    triangles.append(tri);

    Shape::Handle inst{.light_id = InvalidUI32,
                       .mesh_id = (uint)mesh_handles.host().size(),
                       .o2w = o2w};
    instances.push_back(inst);
    mesh_handles.push_back(mesh_handle);
}

void DeviceData::build_meshes() {
    for (const auto &inst : instances) {
        uint mesh_id = inst.mesh_id;
        const auto &mesh_handle = mesh_handles[mesh_id];
        ocarina::Mesh mesh;
        if (mesh_id == mesh_handles.host().size() - 1) {
            // last element
            BufferView<Vertex> verts = vertices.device().view(mesh_handle.vertex_offset, 0);
            BufferView<Triangle> tris = triangles.device().view(mesh_handle.triangle_offset, 0);
            mesh = device->create_mesh(verts, tris);
        } else {
            const auto &next_mesh_handle = mesh_handles[mesh_id + 1];
            uint vert_count = next_mesh_handle.vertex_offset - mesh_handle.vertex_offset;
            uint tri_count = next_mesh_handle.triangle_offset - mesh_handle.triangle_offset;
            BufferView<Vertex> verts = vertices.device().view(mesh_handle.vertex_offset, vert_count);
            BufferView<Triangle> tris = triangles.device().view(mesh_handle.triangle_offset, tri_count);
            mesh = device->create_mesh(verts, tris);
        }
        meshes.push_back(std::move(mesh));
    }
}

void DeviceData::reset_device_buffer() {
    vertices.reset_device_buffer(*device);
    triangles.reset_device_buffer(*device);
    instances.reset_device_buffer(*device);
    mesh_handles.reset_device_buffer(*device);
}

void DeviceData::upload() const {
    Stream stream = device->create_stream();
    stream << vertices.upload()
           << triangles.upload()
           << mesh_handles.upload()
           << instances.upload()
           << synchronize();
    stream << commit();
}

void DeviceData::build_accel() {
    accel = device->create_accel();
    Stream stream = device->create_stream();
    for (int i = 0; i < meshes.size(); ++i) {
        Shape::Handle inst = instances[i];
        ocarina::Mesh &mesh = meshes[i];
        stream << mesh.build_bvh();
        accel.add_mesh(mesh, inst.o2w);
    }
    stream << accel.build_bvh();
    stream << synchronize();
    stream << commit();
}

SurfaceInteraction<D> DeviceData::compute_surface_interaction(const OCHit &hit) const noexcept {
    SurfaceInteraction<D> ret;
    Var inst = instances.read(hit.inst_id);
    Var mesh = mesh_handles.read(inst.mesh_id);
    auto o2w = Transform(inst.o2w);
    Var tri = triangles.read(mesh.triangle_offset + hit.prim_id);
    auto [v0, v1, v2] = get_vertices(tri, mesh.vertex_offset);
    Float3 pos = hit->lerp(v0.position, v1.position, v2.position);
    Float3 normal = hit->lerp(v0.normal, v1.normal, v2.normal);
    Float2 uv = hit->lerp(v0.tex_coord, v1.tex_coord, v2.tex_coord);
    ret.g_uvn.z = normalize(o2w.apply_normal(normal));
    ret.pos = pos;
    ret.uv = uv;
    return ret;
}

array<Var<Vertex>, 3> DeviceData::get_vertices(const Var<Triangle> &tri,
                                               const Uint &offset) const noexcept {
    return {vertices.read(offset + tri.i),
            vertices.read(offset + tri.j),
            vertices.read(offset + tri.k)};
}

}// namespace vision