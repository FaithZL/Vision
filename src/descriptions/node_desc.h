//
// Created by Zero on 10/09/2022.
//

#pragma once

#include <utility>
#include "core/stl.h"
#include "core/hash.h"
#include "core/basic_types.h"
#include "parameter_set.h"
#include "math/geometry.h"

namespace vision {

using namespace ocarina;
class Scene;

struct NameID {
public:
    using map_ty = map<string, uint>;

public:
    string name;
    uint id{InvalidUI32};
    bool valid() { return id != InvalidUI32; }
    void fill_id(const map_ty &name_to_id) {
        if (name_to_id.contains(name)) {
            id = name_to_id.at(name);
        } else {
            id = InvalidUI32;
        }
    }
};

enum AttrType {
    Number,
    Albedo,
    Unbound,
    Illumination
};

struct NodeDesc : public Hashable {
protected:
    string_view _type;

public:
    string sub_type;
    string name;
    mutable Scene *scene{nullptr};
    mutable fs::path scene_path;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(_type, hash64(sub_type));
    }

public:
    NodeDesc() = default;
    NodeDesc(string_view type, string name)
        : _type(type), sub_type(std::move(name)) {}
    explicit NodeDesc(string_view type) : _type(type) {}
    virtual void init(const ParameterSet &ps) noexcept {
        if (ps.data().is_object())
            name = ps["name"].as_string();
    };
    [[nodiscard]] string plugin_name() const noexcept {
        return "vision-" + to_lower(string(_type)) + "-" + to_lower(sub_type);
    }
    [[nodiscard]] virtual bool operator==(const NodeDesc &other) const noexcept {
        return hash() == other.hash();
    }
};
#define VISION_DESC_COMMON(type)      \
    type##Desc() : NodeDesc(#type) {} \
    explicit type##Desc(string name) : NodeDesc(#type, std::move(name)) {}

struct TransformDesc : public NodeDesc {
public:
    float4x4 mat{make_float4x4(1.f)};

public:
    void init(const ParameterSet &ps) noexcept override;
};

struct ShaderNodeDesc : public NodeDesc {
public:
    float4 val{};
    string fn;
    ColorSpace color_space{ColorSpace::SRGB};
    AttrType type{};

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(NodeDesc::_compute_hash(), fn, val, color_space);
    }

public:
    explicit ShaderNodeDesc(AttrType type)
        : NodeDesc("ShaderNode"), type(type) { sub_type = "constant"; }
    explicit ShaderNodeDesc(string name, AttrType type)
        : NodeDesc("ShaderNode", std::move(name)), type(type) {}
    explicit ShaderNodeDesc(float v, AttrType type)
        : NodeDesc("ShaderNode"), val(make_float4(v)), type(type) { sub_type = "constant"; }
    explicit ShaderNodeDesc(float2 v, AttrType type)
        : NodeDesc("ShaderNode"), val(make_float4(v, 0, 0)), type(type) { sub_type = "constant"; }
    explicit ShaderNodeDesc(float3 v, AttrType type)
        : NodeDesc("ShaderNode"), val(make_float4(v, 0)), type(type) { sub_type = "constant"; }
    explicit ShaderNodeDesc(float4 v, AttrType type)
        : NodeDesc("ShaderNode"), val(v), type(type) { sub_type = "constant"; }
    void init(const ParameterSet &ps) noexcept override;
    void init(const ParameterSet &ps, fs::path scene_path) noexcept {
        this->scene_path = scene_path;
        init(ps);
    }
    [[nodiscard]] bool valid_emission() const noexcept {
        return any(val != 0.f) || !fn.empty();
    }
};

struct LightDesc : public NodeDesc {
public:
    // for area light and projector
    ShaderNodeDesc texture_desc{Illumination};

    // area light
    bool two_sided{false};
    float scale{1.f};
    uint inst_id{InvalidUI32};

    // point light
    float3 position;
    float3 intensity{};

    // for spotlight and projector
    float angle{45};
    float falloff{10};
    float3 direction{make_float3(0, 0, 1)};

    // for projector width / height
    float ratio{0.f};

    // for environment and projector
    TransformDesc o2w;

    VISION_DESC_COMMON(Light)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] bool valid() const noexcept {
        return !sub_type.empty();
    }
};

struct ShapeDesc : public NodeDesc {
public:
    TransformDesc o2w;
    LightDesc emission;
    NameID material;
    NameID inside_medium;
    NameID outside_medium;
    uint64_t mat_hash{InvalidUI32};
    uint index{InvalidUI32};
    fs::path fn;
    bool smooth{false};
    bool flip_uv{true};
    bool swap_handed{false};
    uint subdiv_level{0};

    // quad param
    float width{1};
    float height{1};

    // cube param
    float x{1}, y{1}, z{1};

    // sphere param
    float radius{1};
    uint sub_div{60};

    // mesh param
    mutable vector<Vertex> vertices;
    mutable vector<Triangle> triangles;

public:
    VISION_DESC_COMMON(Shape)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] bool operator==(const ShapeDesc &other) const noexcept;
};

struct SamplerDesc : public NodeDesc {
public:
    uint spp{};

public:
    VISION_DESC_COMMON(Sampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilterDesc : public NodeDesc {
public:
    float2 radius{make_float2(1.f)};
    // for gaussian filter
    float sigma{};
    // for sinc filter
    float tau{};
    // for mitchell filter
    float b{}, c{};

public:
    VISION_DESC_COMMON(Filter)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilmDesc : public NodeDesc {
public:
    int state{0};
    int tone_map{0};
    uint2 resolution{make_uint2(500)};

public:
    VISION_DESC_COMMON(Film)
    void init(const ParameterSet &ps) noexcept override;
};

struct SensorDesc : public NodeDesc {
public:
    TransformDesc transform_desc;
    float fov_y{20};
    float velocity{5};
    float sensitivity{0.5};
    float focal_distance{5.f};
    float lens_radius{0.f};
    FilterDesc filter_desc;
    FilmDesc film_desc;
    NameID medium;

public:
    VISION_DESC_COMMON(Sensor)
    void init(const ParameterSet &ps) noexcept override;
};

struct IntegratorDesc : public NodeDesc {
public:
    uint max_depth{10};
    uint min_depth{5};
    float rr_threshold{1};

public:
    VISION_DESC_COMMON(Integrator)
    void init(const ParameterSet &ps) noexcept override;
};

struct MediumDesc : public NodeDesc {
public:
    float3 sigma_a{make_float3(0.f)};
    float3 sigma_s{make_float3(0.8f)};
    float g{0.f};
    float scale{1.f};
    string medium_name;
    uint index{InvalidUI32};
public:
    VISION_DESC_COMMON(Medium)
    void init(const ParameterSet &ps) noexcept override;
};

struct MaterialDesc : public NodeDesc {
public:
    // common
    ShaderNodeDesc color{Albedo};
    ShaderNodeDesc roughness{1.f, Number};
    bool remapping_roughness{false};

    // for glass and disney
    ShaderNodeDesc ior{1.5f, Number};

    // for metal
    string material_name{""};
    ShaderNodeDesc eta{Number};
    ShaderNodeDesc k{Number};

    // for substrate
    ShaderNodeDesc spec{0.05f, Albedo};

    // for disney
    ShaderNodeDesc metallic{0.f, Number};
    ShaderNodeDesc spec_tint{0.0f, Number};
    ShaderNodeDesc anisotropic{0.0f, Number};
    ShaderNodeDesc sheen{0.f, Number};
    ShaderNodeDesc sheen_tint{0.f, Number};
    ShaderNodeDesc clearcoat{0.f, Number};
    ShaderNodeDesc clearcoat_alpha{0.f, Number};
    ShaderNodeDesc spec_trans{0.f, Number};
    ShaderNodeDesc scatter_distance{0.f, Number};
    ShaderNodeDesc flatness{0.f, Number};
    ShaderNodeDesc diff_trans{0.f, Number};
    bool thin{false};

    // for subsurface
    ShaderNodeDesc sigma_a{make_float3(.0011f, .0024f, .014f), Unbound};
    ShaderNodeDesc sigma_s{make_float3(2.55f, 3.21f, 3.77f), Unbound};
    float sigma_scale{1.f};

public:
    VISION_DESC_COMMON(Material)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(NodeDesc::_compute_hash(), color, roughness,
                      remapping_roughness, ior, eta, k, spec, metallic,
                      spec_tint, anisotropic, sheen, sheen_tint, clearcoat,
                      clearcoat_alpha, spec_trans, scatter_distance, flatness,
                      diff_trans, thin);
    }
};

struct LightSamplerDesc : public NodeDesc {
public:
    vector<LightDesc> light_descs;
    float env_prob{0.5f};
    VISION_DESC_COMMON(LightSampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct WarperDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Warper)
    void init(const ParameterSet &ps) noexcept override;
};

struct SpectrumDesc : public NodeDesc {
public:
    uint dimension{3};

public:
    VISION_DESC_COMMON(Spectrum)
    void init(const ParameterSet &ps) noexcept override;
};

struct OutputDesc : public NodeDesc {
public:
    string fn;
    uint spp{0u};
    bool save_exit{false};
    VISION_DESC_COMMON(Output)
    void init(const ParameterSet &ps, fs::path scene_path) noexcept {
        this->scene_path = move(scene_path);
        init(ps);
    }
    void init(const ParameterSet &ps) noexcept override;
};

}// namespace vision