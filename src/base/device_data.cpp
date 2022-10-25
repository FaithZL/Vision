//
// Created by Zero on 25/10/2022.
//

#include "device_data.h"

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
    reset_device_buffer();

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

}// namespace vision