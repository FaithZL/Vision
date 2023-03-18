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

enum ShaderNodeType {
    Number,
    Albedo,
    Unbound,
    Illumination,
    Calculate
};

struct ObjectDesc : public Hashable {
protected:
    string_view _type;
    ParameterSet _parameter{DataWrap::object()};

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
    ObjectDesc() = default;
    ObjectDesc(string_view type, string name)
        : _type(type), sub_type(std::move(name)) {}
    explicit ObjectDesc(string_view type) : _type(type) {}
    [[nodiscard]] string parameter_string() const noexcept;
    [[nodiscard]] ParameterSet operator[](const string &key) const noexcept { return _parameter[key]; }
    template<typename... Args>
    void set_value(Args &&...args) noexcept {
        _parameter.set_value(OC_FORWARD(args)...);
    }
    void set_parameter(const ParameterSet &ps) noexcept;
    virtual void init(const ParameterSet &ps) noexcept {
        if (ps.data().is_object())
            name = ps["name"].as_string();
    };
    [[nodiscard]] string plugin_name() const noexcept {
        return "vision-" + to_lower(string(_type)) + "-" + to_lower(sub_type);
    }
    [[nodiscard]] virtual bool operator==(const ObjectDesc &other) const noexcept {
        return hash() == other.hash();
    }
};
#define VISION_DESC_COMMON(type)      \
    type##Desc() : ObjectDesc(#type) {} \
    explicit type##Desc(string name) : ObjectDesc(#type, std::move(name)) {}

struct TransformDesc : public ObjectDesc {
public:
    float4x4 mat{make_float4x4(1.f)};

public:
    void init(const ParameterSet &ps) noexcept override;
};

struct ShaderNodeDesc : public ObjectDesc {
public:
    ShaderNodeType type{};

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(ObjectDesc::_compute_hash(), parameter_string());
    }

public:
    explicit ShaderNodeDesc(ShaderNodeType type)
        : ObjectDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
    }
    explicit ShaderNodeDesc(string name, ShaderNodeType type)
        : ObjectDesc("ShaderNode", std::move(name)), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
    }
    explicit ShaderNodeDesc(float v, ShaderNodeType type)
        : ObjectDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
        _parameter.set_value("value", {v, v, v, v});
    }
    explicit ShaderNodeDesc(float2 v, ShaderNodeType type)
        : ObjectDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
        _parameter.set_value("value", {v.x, v.y, 0, 0});
    }
    explicit ShaderNodeDesc(float3 v, ShaderNodeType type)
        : ObjectDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
        _parameter.set_value("value", {v.x, v.y, v.z, 0});
    }
    explicit ShaderNodeDesc(float4 v, ShaderNodeType type)
        : ObjectDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
        _parameter.set_value("value", {v.x, v.y, v.z, v.w});
    }
    void init(const ParameterSet &ps) noexcept override;
    void init(const ParameterSet &ps, fs::path scene_path) noexcept {
        this->scene_path = scene_path;
        init(ps);
    }
};

template<uint dim>
requires(dim <= 4) struct SlotDesc : public ObjectDesc {
public:
    static constexpr auto default_channels() noexcept {
        if constexpr (dim == 1) {
            return "x";
        } else if constexpr (dim == 2) {
            return "xy";
        } else if constexpr (dim == 3) {
            return "xyz";
        } else {
            return "xyzw";
        }
    }

public:
    string channels;
    ShaderNodeDesc node;
    VISION_DESC_COMMON(Slot)
    explicit SlotDesc(ShaderNodeDesc node, string channels = default_channels())
        : node(node), channels(channels) {}

    explicit SlotDesc(ShaderNodeType type, string channels = default_channels())
        : node(type), channels(channels) {}

    void init(const ParameterSet &ps) noexcept override {
        DataWrap data = ps.data();
        if (data.contains("channels")) {
            channels = ps["channels"].as_string();
            node.init(ps["node"], scene_path);
        } else {
            node.init(ps, scene_path);
        }
    }
    void init(const ParameterSet &ps, fs::path scene_path) noexcept {
        this->scene_path = scene_path;
        init(ps);
    }
};

struct LightDesc : public ObjectDesc {
public:
    ShaderNodeDesc color_desc{Illumination};

    SlotDesc<3> color_slot{Illumination};

    TransformDesc o2w;

    VISION_DESC_COMMON(Light)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] bool valid() const noexcept {
        return !sub_type.empty();
    }
};

struct ShapeDesc : public ObjectDesc {
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

struct SamplerDesc : public ObjectDesc {
public:
    VISION_DESC_COMMON(Sampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilterDesc : public ObjectDesc {
public:
    VISION_DESC_COMMON(Filter)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilmDesc : public ObjectDesc {
public:
    VISION_DESC_COMMON(Film)
    void init(const ParameterSet &ps) noexcept override;
};

struct SensorDesc : public ObjectDesc {
public:
    TransformDesc transform_desc;
    FilterDesc filter_desc;
    FilmDesc film_desc;
    NameID medium;

public:
    VISION_DESC_COMMON(Sensor)
    void init(const ParameterSet &ps) noexcept override;
};

struct IntegratorDesc : public ObjectDesc {
public:
    VISION_DESC_COMMON(Integrator)
    void init(const ParameterSet &ps) noexcept override;
};

struct MediumDesc : public ObjectDesc {
public:
    ShaderNodeDesc sigma_a{Unbound};
    ShaderNodeDesc sigma_s{Unbound};
    ShaderNodeDesc g{Number};
    ShaderNodeDesc scale{Number};

public:
    VISION_DESC_COMMON(Medium)
    void init(const ParameterSet &ps) noexcept override;
};

struct MaterialDesc : public ObjectDesc {
public:
    VISION_DESC_COMMON(Material)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] ShaderNodeDesc attr(const string &key, auto default_value,
                                      ShaderNodeType type = ShaderNodeType::Number) const noexcept {
        ShaderNodeDesc ret{default_value, type};
        ret.init(_parameter[key], scene_path);
        return ret;
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;
};

struct LightSamplerDesc : public ObjectDesc {
public:
    vector<LightDesc> light_descs;
    VISION_DESC_COMMON(LightSampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct WarperDesc : public ObjectDesc {
public:
    VISION_DESC_COMMON(Warper)
    void init(const ParameterSet &ps) noexcept override;
};

struct SpectrumDesc : public ObjectDesc {
public:
    VISION_DESC_COMMON(Spectrum)
    void init(const ParameterSet &ps) noexcept override;
};

struct OutputDesc : public ObjectDesc {
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