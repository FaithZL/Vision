//
// Created by Zero on 25/10/2022.
//

#include "geometry.h"
#include "math/transform.h"
#include "scene.h"
#include "base/scattering/medium.h"
#include "pipeline.h"
#include "mesh_registry.h"

namespace vision {

Geometry::Geometry(Pipeline *rp)
    : rp(rp), vertices_(rp->bindless_array()),
      triangles_(rp->bindless_array()),
      instances_(rp->bindless_array()),
      mesh_handles_(rp->bindless_array()),
      accel_(rp->device().create_accel()) {}

void Geometry::update_instances(const vector<SP<ShapeInstance>> &instances) {

    vertices_.host_buffer().clear();
    triangles_.host_buffer().clear();
    instances_.host_buffer().clear();
    mesh_handles_.host_buffer().clear();

    MeshRegistry::instance().for_each([&](const Mesh *mesh, uint i) {
        MeshHandle mesh_handle{.vertex_offset = (uint)vertices_.host_buffer().size(),
                               .triangle_offset = (uint)triangles_.host_buffer().size(),
                               .vertex_buffer = mesh->vertex_buffer().index().hv(),
                               .triangle_buffer = mesh->triangle_buffer().index().hv()};
        vertices_.append(mesh->vertices());
        triangles_.append(mesh->triangles());
        mesh_handles_.push_back(mesh_handle);
    });

    std::for_each(instances.begin(), instances.end(), [&](SP<const ShapeInstance> instance) {
        instances_.push_back(instance->handle());
    });
}

void Geometry::build_accel() {
    Stream &stream = rp->stream();
    for (const auto &inst : instances_) {
        uint mesh_id = inst.mesh_id;
        const auto &mesh_handle = mesh_handles_[mesh_id];
        ocarina::RHIMesh mesh;
        if (mesh_id == mesh_handles_.host_buffer().size() - 1) {
            // last element
            BufferView<Vertex> verts = vertices_.device_buffer().view(mesh_handle.vertex_offset, 0);
            BufferView<Triangle> tris = triangles_.device_buffer().view(mesh_handle.triangle_offset, 0);
            mesh = rp->device().create_mesh(verts, tris);
        } else {
            const auto &next_mesh_handle = mesh_handles_[mesh_id + 1];
            uint vert_count = next_mesh_handle.vertex_offset - mesh_handle.vertex_offset;
            uint tri_count = next_mesh_handle.triangle_offset - mesh_handle.triangle_offset;
            BufferView<Vertex> verts = vertices_.device_buffer().view(mesh_handle.vertex_offset, vert_count);
            BufferView<Triangle> tris = triangles_.device_buffer().view(mesh_handle.triangle_offset, tri_count);
            mesh = rp->device().create_mesh(verts, tris);
        }
        stream << mesh.build_bvh();
        accel_.add_instance(ocarina::move(mesh), inst.o2w());
    }

    OC_INFO_FORMAT("vertex num is {}, triangle num is {}", accel_.vertex_num(), accel_.triangle_num());
    stream << accel_.build_bvh();
    stream << synchronize();
    stream << commit();
}

void Geometry::reset_device_buffer() {
    auto init_buffer = [&](auto &buffer, const string &desc) {
        buffer.reset_device_buffer_immediately(rp->device(), desc);
        buffer.register_self();
    };
    init_buffer(vertices_, "Geometry::vertices_");
    init_buffer(triangles_, "Geometry::triangles_");
    init_buffer(instances_, "Geometry::instances_");
    init_buffer(mesh_handles_, "Geometry::mesh_handles_");
}

void Geometry::upload() const {
    Stream &stream = rp->stream();
    stream << vertices_.upload()
           << triangles_.upload()
           << MeshRegistry::instance().upload_meshes()
           << mesh_handles_.upload()
           << instances_.upload()
           << synchronize();
    stream << commit();
}

void Geometry::clear() noexcept {
    vertices_.clear_all();
    triangles_.clear_all();
    instances_.clear_all();
    mesh_handles_.clear_all();
    accel_.clear();
}

Interaction Geometry::compute_surface_interaction(const TriangleHitVar &hit, bool is_complete) const noexcept {
    Interaction it{Global::instance().pipeline()->scene().has_medium()};
    it.prim_id = hit.prim_id;
    Var inst = instances_.read(hit.inst_id);
    Var mesh = mesh_handles_.read(inst.mesh_id);
    auto o2w = Transform(inst->o2w());
    Var tri = triangles_.read(mesh.triangle_offset + hit.prim_id);
    auto [v0, v1, v2] = get_vertices(tri, mesh.vertex_offset);
    it.lightmap_id = inst.lightmap_id;
    it.set_light(inst.light_id);
    it.set_material(inst.mat_id);
    it.set_medium(inst.inside_medium, inst.outside_medium);
    comment("compute pos");
    outline([&] {
        Var p0 = o2w.apply_point(v0->position());
        Var p1 = o2w.apply_point(v1->position());
        Var p2 = o2w.apply_point(v2->position());
        Float3 pos = hit->triangle_lerp(p0, p1, p2);
        it.pos = pos;
        it.lightmap_uv = hit->triangle_lerp(v0->lightmap_uv(), v1->lightmap_uv(), v2->lightmap_uv());

        Frame<Float3> frame;

        comment("compute geometry uvn");
        Float3 dp02 = p0 - p2;
        Float3 dp12 = p1 - p2;
        Float3 ng_un = cross(dp02, dp12);
        it.prim_area = 0.5f * length(ng_un);
        Float2 duv02 = v0->tex_coord() - v2->tex_coord();
        Float2 duv12 = v1->tex_coord() - v2->tex_coord();
        Float det = duv02[0] * duv12[1] - duv02[1] * duv12[0];
        Bool degenerate_uv = abs(det) < float(1e-8);
        if (is_complete) {
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
            frame.set(dp_du, dp_dv, normalize(ng_un));
        } else {
            frame.set(dp02, dp12, normalize(ng_un));
        }

        if (is_complete) {
            comment("compute shading uvn");
            Float3 dn_du, dn_dv;
            Float3 normal = hit->triangle_lerp(v0->normal(), v1->normal(), v2->normal());
            it.shading.set_frame(frame);

            $if(!is_zero(normal)) {
                Float3 ns = normalize(o2w.apply_normal(normal));
                it.shading.update(ns);
            };

            Float3 dn1 = v0->normal() - v2->normal();
            Float3 dn2 = v1->normal() - v2->normal();
            Float3 dn = cross(v2->normal() - v0->normal(),
                              v1->normal() - v0->normal());

            $if(degenerate_uv) {
                $if(length_squared(dn)) {
                    dn_du = make_float3(0.f);
                    dn_dv = make_float3(0.f);
                }
                $else {
                    coordinate_system(dn, dn_du, dn_dv);
                };
            }
            $else {
                Float inv_det = 1 / det;
                dn_du = (duv12[1] * dn1 - duv02[1] * dn2) * inv_det;
                dn_dv = (-duv12[0] * dn1 + duv02[0] * dn2) * inv_det;
            };
            it.shading.dn_du = dn_du;
            it.shading.dn_dv = dn_dv;
        }
        Float2 uv = hit->triangle_lerp(v0->tex_coord(), v1->tex_coord(), v2->tex_coord());
        it.uv = uv;
        it.ng = frame.z;
    },
            "Geometry::compute_surface_interaction");
    return it;
}

TriangleHitVar Geometry::trace_closest(const RayVar &ray) const noexcept {
    return accel_.trace_closest(ray);
}
Bool Geometry::trace_occlusion(const RayVar &ray) const noexcept {
    return accel_.trace_occlusion(ray);
}

Bool Geometry::occluded(const Interaction &it, const Float3 &pos, RayState *rs) const noexcept {
    RayVar shadow_ray;
    if (rs) {
        *rs = it.spawn_ray_state_to(pos);
        shadow_ray = rs->ray;
    } else {
        shadow_ray = it.spawn_ray_to(pos);
    }
    return trace_occlusion(shadow_ray);
}

SampledSpectrum Geometry::Tr(Scene &scene, const SampledWavelengths &swl,
                             const RayState &ray_state) const noexcept {
    TSampler &sampler = scene.sampler();
    SampledSpectrum ret{swl.dimension(), 1.f};
    if (scene.has_medium()) {
        $if(ray_state.in_medium()) {
            scene.mediums().dispatch_instance(ray_state.medium, [&](const Medium *medium) {
                ret = medium->Tr(ray_state.ray, swl, sampler);
            });
        };
    }
    return ret;
}

array<Var<Vertex>, 3> Geometry::get_vertices(const Var<Triangle> &tri,
                                             const Uint &offset) const noexcept {
    return {vertices_.read(offset + tri.i),
            vertices_.read(offset + tri.j),
            vertices_.read(offset + tri.k)};
}
LightEvalContext Geometry::compute_light_eval_context(const Uint &inst_id,
                                                      const Uint &prim_id,
                                                      const Float2 &bary) const noexcept {
    TriangleHitVar hit;
    hit.inst_id = inst_id;
    hit.prim_id = prim_id;
    hit.bary = bary;
    Interaction it = compute_surface_interaction(hit, false);
    LightEvalContext ret(it);
    return ret;
}

}// namespace vision