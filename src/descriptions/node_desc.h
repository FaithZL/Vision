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
    ParameterSet _parameter;

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
    [[nodiscard]] ParameterSet operator[](const string &key) const noexcept { return _parameter[key]; }
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
    VISION_DESC_COMMON(Sampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilterDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Filter)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilmDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Film)
    void init(const ParameterSet &ps) noexcept override;
};

struct SensorDesc : public NodeDesc {
public:
    TransformDesc transform_desc;
    FilterDesc filter_desc;
    FilmDesc film_desc;
    NameID medium;

public:
    VISION_DESC_COMMON(Sensor)
    void init(const ParameterSet &ps) noexcept override;
};

struct IntegratorDesc : public NodeDesc {
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
    VISION_DESC_COMMON(Material)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] ShaderNodeDesc attr(const string &key, auto default_value,
                                      AttrType type = AttrType::Number) const noexcept {
        ShaderNodeDesc ret{default_value, type};
        ret.init(_parameter[key], scene_path);
        return ret;
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(NodeDesc::_compute_hash(), _parameter.as_string());
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