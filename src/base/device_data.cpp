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

SurfaceInteraction DeviceData::compute_surface_interaction(const OCHit &hit) const noexcept {
    SurfaceInteraction si;
    Var inst = instances.read(hit.inst_id);
    Var mesh = mesh_handles.read(inst.mesh_id);
    auto o2w = Transform(inst.o2w);
    Var tri = triangles.read(mesh.triangle_offset + hit.prim_id);
    auto [v0, v1, v2] = get_vertices(tri, mesh.vertex_offset);

    {
        $comment(compute pos)
            Var p0 = o2w.apply_point(v0.pos);
        Var p1 = o2w.apply_point(v1.pos);
        Var p2 = o2w.apply_point(v2.pos);
        Float3 pos = hit->lerp(p0, p1, p2);
        si.pos = pos;

        $comment(compute geometry uvn)
            Float3 dp02 = p0 - p2;
        Float3 dp12 = p1 - p2;
        Float3 ng_un = cross(dp02, dp12);
        si.prim_area = 0.5f * length(ng_un);
        Float2 duv02 = v0.uv - v2.uv;
        Float2 duv12 = v1.uv - v2.uv;
        Float det = duv02[0] * duv12[1] - duv02[1] * duv12[0];
        Bool degenerate_uv = abs(det) < float(1e-8);
        Float3 dp_du, dp_dv;

        $if(!degenerate_uv) {
            Float inv_det = 1 / det;
            dp_du = (duv12[1] * dp02 - duv02[1] * dp12) * inv_det;
            dp_dv = (-duv12[0] * dp02 + duv02[0] * dp12) * inv_det;
        }
        $else {
            dp_du = normalize(p1 - p0);
            dp_dv = normalize(p2 - p0);
        };
        Float3 ng = normalize(ng_un);
        Float3 normal = hit->lerp(v0.normal, v1.normal, v2.normal);

        si.g_uvn.set(normalize(dp_du), normalize(dp_dv), normalize(ng_un));
    }

    {
        $comment(compute shading uvn)
            Float3 normal = hit->lerp(v0.normal, v1.normal, v2.normal);
        $if(is_zero(normal)) {
            si.s_uvn = si.g_uvn;
        } $else {
            Float3 ns = normalize(o2w.apply_normal(normal));
            Float3 ss = si.g_uvn.dp_du();
            Float3 st = normalize(cross(ns, ss));
            ss = cross(st, ns);
            si.s_uvn.set(ss, st, ns);
        };
    }
    Float2 uv = hit->lerp(v0.uv, v1.uv, v2.uv);
    si.uv = uv;
    return si;
}

array<Var<Vertex>, 3> DeviceData::get_vertices(const Var<Triangle> &tri,
                                               const Uint &offset) const noexcept {
    return {vertices.read(offset + tri.i),
            vertices.read(offset + tri.j),
            vertices.read(offset + tri.k)};
}
LightEvalContext DeviceData::compute_light_eval_context(const Uint &inst_id,
                                                        const Uint &prim_id,
                                                        const Float2 &bary) const noexcept {
    OCHit hit;
    hit.inst_id = inst_id;
    hit.prim_id = prim_id;
    hit.bary = bary;
    // todo
    SurfaceInteraction si = compute_surface_interaction(hit);
    LightEvalContext ret(si);
    ret.PDF_pos = 1 / si.prim_area;
    return ret;
}

}// namespace vision