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

struct InstanceHandle {
    // todo compress unsigned int data
    uint light_id{InvalidUI32};
    uint mat_id{InvalidUI32};
    uint lightmap_id{InvalidUI32};
    uint mesh_id{InvalidUI32};
    uint inside_medium{InvalidUI32};
    uint outside_medium{InvalidUI32};
    float4x4 o2w;
};

}// namespace vision

OC_STRUCT(vision::InstanceHandle, light_id, mat_id, lightmap_id,
          mesh_id, inside_medium, outside_medium, o2w){};

namespace vision {
struct Mesh : public Hashable {
public:
    struct Handle {
        uint vertex_offset;
        uint triangle_offset;
    };

protected:
    vector<Vertex> _vertices;
    vector<Triangle> _triangles;
    uint _index{};

    uint2 _resolution;
    // auto unwrap light map uv is not normalized
    bool _normalized{false};
    // after unwrap light map uv, mesh vertices layout maybe has changed
    bool _cleanup{false};

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;

public:
    Mesh(vector<Vertex> vert, vector<Triangle> tri)
        : _vertices(std::move(vert)), _triangles(std::move(tri)) {}
    Mesh() = default;
    OC_MAKE_MEMBER_GETTER_SETTER(index, )
    OC_MAKE_MEMBER_GETTER_SETTER(vertices, &)
    OC_MAKE_MEMBER_GETTER_SETTER(triangles, &)
    OC_MAKE_MEMBER_GETTER_SETTER(cleanup, )
    OC_MAKE_MEMBER_GETTER_SETTER(resolution, )
    void normalize_lightmap_uv(uint2 res) noexcept;
    [[nodiscard]] Box3f compute_aabb() const noexcept;
    [[nodiscard]] uint lightmap_size() const noexcept;
    [[nodiscard]] vector<float> surface_areas() const noexcept;
};

}// namespace vision

OC_STRUCT(vision::Mesh::Handle, vertex_offset, triangle_offset){};

#define VS_MAKE_ATTR_SETTER_GETTER(attr)                     \
    void set_##attr(decltype(_##attr.object) val) noexcept { \
        _##attr.object = val;                                \
    }                                                        \
    void set_##attr##_name(const string &name) noexcept {    \
        _##attr.name = name;                                 \
    }                                                        \
    void set_##attr(decltype(_##attr) val) noexcept {        \
        _##attr = val;                                       \
    }                                                        \
    [[nodiscard]] auto attr() const noexcept {               \
        return _##attr.object;                               \
    }                                                        \
    [[nodiscard]] auto attr() noexcept {                     \
        return _##attr.object;                               \
    }                                                        \
    [[nodiscard]] bool has_##attr() const noexcept {         \
        return bool(attr());                                 \
    }                                                        \
    [[nodiscard]] string attr##_name() const noexcept {      \
        return _##attr.name;                                 \
    }

namespace vision {
class ShapeInstance {
protected:
    InstanceHandle _handle;
    float _factor{};
    uint _index{};
    Wrap<IAreaLight> _emission{};
    Wrap<Material> _material{};
    Wrap<Medium> _inside{};
    Wrap<Medium> _outside{};
    SP<Mesh> _mesh{};
    string _name;

public:
    Box3f aabb;

public:
    explicit ShapeInstance(SP<Mesh> mesh);
    explicit ShapeInstance(Mesh mesh);
    OC_MAKE_MEMBER_GETTER_SETTER(index, )
    OC_MAKE_MEMBER_GETTER_SETTER(mesh, )
    OC_MAKE_MEMBER_GETTER_SETTER(name, )
    OC_MAKE_MEMBER_GETTER_SETTER(handle, &)
    void fill_mesh_id() noexcept;
    [[nodiscard]] Box3f compute_aabb() const noexcept;
    void init_aabb() noexcept { aabb = compute_aabb(); }
    VS_MAKE_ATTR_SETTER_GETTER(inside)
    VS_MAKE_ATTR_SETTER_GETTER(outside)
    VS_MAKE_ATTR_SETTER_GETTER(material)
    VS_MAKE_ATTR_SETTER_GETTER(emission)
    void set_lightmap_id(uint id) noexcept { _handle.lightmap_id = id; }
    [[nodiscard]] float4x4 o2w() const noexcept { return _handle.o2w; }
    void set_o2w(float4x4 o2w) noexcept { _handle.o2w = o2w; }
    virtual void update_inside_medium_id(uint id) noexcept { _handle.inside_medium = id; }
    virtual void update_outside_medium_id(uint id) noexcept { _handle.outside_medium = id; }
    virtual void update_material_id(uint id) noexcept { _handle.mat_id = id; }
    virtual void update_light_id(uint id) noexcept { _handle.light_id = id; }
    [[nodiscard]] vector<float> surface_areas() const noexcept;
    [[nodiscard]] bool has_lightmap() const noexcept { return _handle.lightmap_id != InvalidUI32; }
};
}// namespace vision

namespace vision {

class ShapeGroup : public Node {
public:
    using Desc = ShapeDesc;

public:
    Box3f aabb;

private:
    vector<ShapeInstance> _instances;

protected:
    Wrap<IAreaLight> _emission{};
    Wrap<Material> _material{};

public:
    ShapeGroup() = default;
    explicit ShapeGroup(ShapeInstance inst);
    explicit ShapeGroup(const ShapeDesc &desc);
    VS_MAKE_ATTR_SETTER_GETTER(material)
    VS_MAKE_ATTR_SETTER_GETTER(emission)
    void post_init(const ShapeDesc &desc);
    [[nodiscard]] ShapeInstance &instance(uint i) noexcept { return _instances[i]; }
    [[nodiscard]] const ShapeInstance &instance(uint i) const noexcept { return _instances[i]; }
    void add_instance(const ShapeInstance &instance) noexcept;
    void add_instances(const vector<ShapeInstance> &instances) noexcept;
    void for_each(const std::function<void(const ShapeInstance &, uint)> &func) const noexcept {
        for (uint i = 0; i < _instances.size(); ++i) {
            func(_instances[i], i);
        }
    }
    void for_each(const std::function<void(ShapeInstance &, uint)> &func) noexcept {
        for (uint i = 0; i < _instances.size(); ++i) {
            func(_instances[i], i);
        }
    }
};

#undef VS_MAKE_ATTR_SETTER_GETTER

}// namespace vision