//
// Created by Zero on 14/09/2022.
//

#pragma once

#include "core/stl.h"
#include "core/basic_types.h"
#include "dsl/rtx_type.h"
#include "node.h"
#include "scattering/medium.h"
#include "math/box.h"
#include "core/vs_header.h"
#include "base/scattering/material.h"
#include "base/illumination/light.h"

namespace vision {

struct Geometry;
struct Mesh;

struct Shape : public Node, public std::enable_shared_from_this<Shape> {
public:
    using Desc = ShapeDesc;

public:
    struct Handle {
        // todo compress unsigned int data
        uint light_id{InvalidUI32};
        uint mat_id{InvalidUI32};
        uint lightmap_id{InvalidUI32};
        uint mesh_id{InvalidUI32};
        uint inside_medium{InvalidUI32};
        uint outside_medium{InvalidUI32};
        float4x4 o2w;
    };

public:
    Box3f aabb;
    Wrap<IAreaLight> _emission{};
    Wrap<Material> _material{};
    Wrap<Medium> _inside{};
    Wrap<Medium> _outside{};

protected:
    Handle _handle;
    float _factor{};
    uint _index{};

protected:
    void init_medium(const ShapeDesc &desc) noexcept;

public:
    explicit Shape(const ShapeDesc &desc);
    Shape() = default;
    virtual void fill_geometry(Geometry &data) const noexcept {
        OC_ASSERT(false);
        OC_ERROR("fill_geometry can not called by model");
    }
    OC_MAKE_MEMBER_GETTER_SETTER(index, )
    void set_emission(const SP<IAreaLight> &light) noexcept {
        _emission.name = light->name();
        _emission.object = light;
    }
    virtual void update_inside_medium_id(uint id) noexcept { _handle.inside_medium = id; }
    virtual void update_outside_medium_id(uint id) noexcept { _handle.outside_medium = id; }
    virtual void update_material_id(uint id) noexcept { _handle.mat_id = id; }
    virtual void update_light_id(uint id) noexcept { _handle.light_id = id; }
    [[nodiscard]] bool has_material() const noexcept { return _material.object.get(); }
    [[nodiscard]] bool has_inside_medium() const noexcept { return _inside.object.get(); }
    [[nodiscard]] bool has_outside_medium() const noexcept { return _outside.object.get(); }
    [[nodiscard]] bool has_lightmap() const noexcept { return _handle.lightmap_id != InvalidUI32; }
    [[nodiscard]] virtual vector<float> ref_surface_areas() const noexcept {
        OC_ASSERT(false);
        OC_ERROR("ref_surface_areas can not called by model");
    }
    [[nodiscard]] virtual uint lightmap_size() const noexcept;
    OC_MAKE_MEMBER_GETTER_SETTER(handle, &)
    virtual void set_lightmap_id(uint id) noexcept {
        OC_ASSERT(false);
        OC_ERROR("set_lightmap_id can not called by model");
    }
    [[nodiscard]] bool has_emission() const noexcept { return _emission.object.get(); }
    [[nodiscard]] virtual vector<float> surface_areas() const noexcept {
        OC_ASSERT(false);
        OC_ERROR("surface_areas can not called by model");
    }
    virtual void for_each_mesh(const std::function<void(SP<Mesh>, uint)> &func) noexcept = 0;
    virtual void for_each_mesh(const std::function<void(SP<const Mesh>, uint)> &func) const noexcept = 0;
    [[nodiscard]] virtual Mesh &mesh_at(uint i) noexcept = 0;
    [[nodiscard]] virtual const Mesh &mesh_at(uint i) const noexcept = 0;
    [[nodiscard]] virtual float4x4 o2w() const noexcept { return _handle.o2w; }
};

}// namespace vision

OC_STRUCT(vision::Shape::Handle, light_id, mat_id, lightmap_id,
          mesh_id, inside_medium, outside_medium, o2w){};

namespace vision {

class Instance {
public:
    struct Handle {
        // todo compress unsigned int data
        uint light_id{InvalidUI32};
        uint mat_id{InvalidUI32};
        uint lightmap_id{InvalidUI32};
        uint mesh_id{InvalidUI32};
        uint inside_medium{InvalidUI32};
        uint outside_medium{InvalidUI32};
        float4x4 o2w;
    };

private:
    Mesh *_mesh{};
    Wrap<IAreaLight> _emission{};
    Wrap<Material> _material{};
    Wrap<Medium> _inside{};
    Wrap<Medium> _outside{};
    Handle _handle;

public:
    Box3f aabb;

public:
    Instance() = default;
    virtual void fill_geometry(Geometry &data) const noexcept {
        OC_ASSERT(false);
        OC_ERROR("fill_geometry can not called by model");
    }
    void set_emission(const SP<IAreaLight> &light) noexcept {
        _emission.name = light->name();
        _emission.object = light;
    }
    void set_material(const SP<Material> &m) noexcept {
        _material.name = m->name();
        _material.object = m;
    }
    void set_inside(const SP<Medium> &inside) noexcept {
        _inside.name = inside->name();
        _inside.object = inside;
    }
    void set_outside(const SP<Medium> &outside) noexcept {
        _outside.name = outside->name();
        _outside.object = outside;
    }
    virtual void update_inside_medium_id(uint id) noexcept { _handle.inside_medium = id; }
    virtual void update_outside_medium_id(uint id) noexcept { _handle.outside_medium = id; }
    virtual void update_material_id(uint id) noexcept { _handle.mat_id = id; }
    virtual void update_light_id(uint id) noexcept { _handle.light_id = id; }
    [[nodiscard]] const Material *material() const noexcept { return _material.object.get(); }
    [[nodiscard]] const IAreaLight *emission() const noexcept { return _emission.object.get(); }
    [[nodiscard]] const Medium *inside_medium() const noexcept { return _inside.object.get(); }
    [[nodiscard]] const Medium *outside_medium() const noexcept { return _outside.object.get(); }
    [[nodiscard]] bool has_material() const noexcept { return _material.object.get(); }
    [[nodiscard]] bool has_inside_medium() const noexcept { return _inside.object.get(); }
    [[nodiscard]] bool has_outside_medium() const noexcept { return _outside.object.get(); }
    [[nodiscard]] bool has_lightmap() const noexcept { return _handle.lightmap_id != InvalidUI32; }
    OC_MAKE_MEMBER_GETTER_SETTER(handle, )
    virtual void set_lightmap_id(uint id) noexcept {
        _handle.lightmap_id = id;
    }
};

}// namespace vision

OC_STRUCT(vision::Instance::Handle, light_id, mat_id, lightmap_id,
          mesh_id, inside_medium, outside_medium, o2w){};

namespace vision {
struct Mesh : public Shape {
public:
    struct Handle {
        uint vertex_offset;
        uint triangle_offset;
    };

public:
    vector<Vertex> vertices;
    vector<Triangle> triangles;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;

public:
    explicit Mesh(const ShapeDesc &desc);
    Mesh(vector<Vertex> vert, vector<Triangle> tri)
        : vertices(std::move(vert)), triangles(std::move(tri)) {}
    Mesh() = default;
    [[nodiscard]] Box3f compute_aabb() const noexcept;
    void init_aabb() noexcept { aabb = compute_aabb(); }
    void fill_geometry(Geometry &data) const noexcept override;
    [[nodiscard]] vector<float> surface_areas() const noexcept override;
    [[nodiscard]] vector<float> ref_surface_areas() const noexcept override;
    void set_lightmap_id(ocarina::uint id) noexcept override { handle().lightmap_id = id; }
    void for_each_mesh(const std::function<void(SP<Mesh>, uint)> &func) noexcept override {
        func(std::dynamic_pointer_cast<Mesh>(shared_from_this()), 0);
    }
    void for_each_mesh(const std::function<void(SP<const Mesh>, uint)> &func) const noexcept override {
        func(std::dynamic_pointer_cast<const Mesh>(shared_from_this()), 0);
    }
    [[nodiscard]] const Mesh &mesh_at(ocarina::uint i) const noexcept override {
        return *this;
    }
    [[nodiscard]] Mesh &mesh_at(ocarina::uint i) noexcept override {
        return *this;
    }
};

}// namespace vision

OC_STRUCT(vision::Mesh::Handle, vertex_offset, triangle_offset){};

namespace vision {

class Group : public Shape {
protected:
    vector<SP<Mesh>> _meshes;
    Box3f _aabb;
    float4x4 _o2w;

public:
    explicit Group(const ShapeDesc &desc)
        : Shape(desc),_o2w(desc.o2w.mat) {}

    [[nodiscard]] Mesh &mesh_at(ocarina::uint i) noexcept override {
        return *_meshes[i];
    }

    [[nodiscard]] const Mesh &mesh_at(ocarina::uint i) const noexcept override {
        return *_meshes[i];
    }

    void for_each_mesh(const std::function<void(SP<Mesh>, uint)> &func) noexcept override {
        for (uint i = 0; i < _meshes.size(); ++i) {
            func(_meshes[i], i);
        }
    }

    void for_each_mesh(const std::function<void(SP<const Mesh>, uint)> &func) const noexcept override {
        for (uint i = 0; i < _meshes.size(); ++i) {
            func(_meshes[i], i);
        }
    }
};

}// namespace vision