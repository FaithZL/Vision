//
// Created by Zero on 10/09/2022.
//

#pragma once

#include <utility>
#include "core/stl.h"
#include "core/basic_types.h"
#include "parameter_set.h"
#include "math/geometry.h"

namespace vision {

using namespace ocarina;

struct Description {
protected:
    string_view _type;

public:
    string sub_type;
    string name;

public:
    Description() = default;
    Description(string_view type, string name)
        : _type(type), sub_type(std::move(name)) {}
    explicit Description(string_view type) : _type(type) {}
    virtual void init(const ParameterSet &ps) noexcept = 0;
};
#define VISION_DESC_COMMON(type)         \
    type##Desc() : Description(#type) {} \
    explicit type##Desc(string name) : Description(#type, std::move(name)) {}

struct TransformDesc : public Description {
public:
    float4x4 mat{make_float4x4(1.f)};
public:
    void init(const ParameterSet &ps) noexcept override;
};

struct ShapeDesc : public Description {
public:
    TransformDesc o2w;
    float3 emission{make_float3(0.f)};
    string material_name;
    string fn;
    bool smooth{false};
    bool swap_handed{};
    uint subdiv_level{};

    // quad param
    float width{};
    float height{};

    // cube param
    float x{1},y{1},z{1};

    // sphere param
    float radius;

    // mesh param
    mutable vector<Vertex> vertices;
    mutable vector<Triangle> triangles;
public:
    VISION_DESC_COMMON(Shape)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] bool operator == (const ShapeDesc &other) const noexcept;
};

struct SamplerDesc : public Description {
public:
    uint spp{};

public:
    VISION_DESC_COMMON(Sampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilterDesc : public Description {
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

struct FilmDesc : public Description {
public:
    int state{0};
    int tone_map{0};
    uint2 resolution{make_uint2(500)};

public:
    VISION_DESC_COMMON(Film)
    void init(const ParameterSet &ps) noexcept override;
};

struct SensorDesc : public Description {
public:
    TransformDesc transform_desc;
    float fov_y{20};
    float velocity{5};
    float focal_distance{5.f};
    float lens_radius{0.f};
    FilterDesc filter_desc;
    FilmDesc film_desc;

public:
    VISION_DESC_COMMON(Sensor)
    void init(const ParameterSet &ps) noexcept override;
};

struct IntegratorDesc : public Description {
public:
    uint max_depth{10};
    uint min_depth{5};
    float rr_threshold{1};

public:
    VISION_DESC_COMMON(Integrator)
    void init(const ParameterSet &ps) noexcept override;
};

struct TextureDesc : public Description {
public:
    float4 val;
    string fn;

public:
    VISION_DESC_COMMON(Texture)
    void init(const ParameterSet &ps) noexcept override;
};

struct MaterialDesc : public Description {
public:
    TextureDesc color;

public:
    VISION_DESC_COMMON(Material)
    void init(const ParameterSet &ps) noexcept override;
};

struct LightDesc : public Description {
    VISION_DESC_COMMON(Light)
    void init(const ParameterSet &ps) noexcept override;
};

struct LightSamplerDesc : public Description {
    VISION_DESC_COMMON(LightSampler)
    void init(const ParameterSet &ps) noexcept override;
};

}// namespace vision