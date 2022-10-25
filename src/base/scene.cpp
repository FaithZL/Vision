//
// Created by Zero on 06/09/2022.
//

#include "scene.h"
#include "core/context.h"

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

    for (int i = 0; i < mesh_handles.host().size(); ++i) {
        const auto &mesh_handle = mesh_handles[i];
        ocarina::Mesh mesh;
        if (i == mesh_handles.host().size() - 1) {
            // last element
            BufferView<Vertex> verts = vertices.device().view(mesh_handle.vertex_offset, 0);
            BufferView<Triangle> tris = triangles.device().view(mesh_handle.triangle_offset, 0);
            mesh = device->create_mesh(verts, tris);
        } else {
            const auto &next_mesh_handle = mesh_handles[i + 1];
            uint vert_count = next_mesh_handle.vertex_offset - mesh_handle.vertex_offset;
            uint tri_count = next_mesh_handle.triangle_offset - mesh_handle.triangle_offset;
            BufferView<Vertex> verts = vertices.device().view(mesh_handle.vertex_offset, vert_count);
            BufferView<Triangle> tris = triangles.device().view(mesh_handle.triangle_offset, tri_count);
            mesh = device->create_mesh(verts, tris);
        }
        meshes.push_back(std::move(mesh));
    }
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
    Stream stream = device->create_stream();
    for (auto &mesh : meshes) {
        stream << mesh.build_bvh();
    }

}

Scene::Scene(vision::Context *ctx)
    : _context(ctx) {}

Node *Scene::load_node(const NodeDesc &desc) {
    const DynamicModule *module = _context->obtain_module(desc.plugin_name());
    auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
    auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
    _all_nodes.emplace_back(creator(desc), deleter);
    return _all_nodes.back().get();
}

void Scene::init(const SceneDesc& scene_desc) {
    TIMER(init_scene);
    scene_desc.light_sampler_desc.scene = this;
    _light_sampler = load<LightSampler>(scene_desc.light_sampler_desc);
    scene_desc.sensor_desc.scene = this;
    _camera = load<Camera>(scene_desc.sensor_desc);
    load_materials(scene_desc.material_descs);
    load_shapes(scene_desc.shape_descs);
    _integrator = load<Integrator>(scene_desc.integrator_desc);
    _sampler = load<Sampler>(scene_desc.sampler_desc);
}

void Scene::prepare(RenderPipeline *rp) noexcept {
    _camera->prepare(rp);
    _sampler->prepare(rp);
    _camera->update_device_data();
}

void Scene::load_materials(const vector<MaterialDesc> &material_descs) noexcept {
    for (const MaterialDesc &desc : material_descs) {
        desc.scene = this;
        _materials.push_back(load<Material>(desc));
    }
}

void Scene::load_shapes(const vector<ShapeDesc> &descs) noexcept {
    for (const ShapeDesc &desc : descs) {
        desc.scene = this;
        auto shape = load<Shape>(desc);
        _aabb.extend(shape->aabb);
        _shapes.push_back(shape);
    }
}

Light *Scene::load_light(const LightDesc &desc) noexcept {
    OC_ASSERT(_light_sampler != nullptr);
    auto ret = load<Light>(desc);
    _light_sampler->add_light(ret);
    return ret;
}

void Scene::load_lights(const vector<LightDesc> &descs) noexcept {
    for (const LightDesc &desc : descs) {
        desc.scene = this;
        load_light(desc);
    }
}

}// namespace vision