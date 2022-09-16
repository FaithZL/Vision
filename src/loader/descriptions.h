//
// Created by Zero on 10/09/2022.
//

#pragma once

#include <utility>

#include "core/stl.h"
#include "core/basic_types.h"
#include "parameter_set.h"

namespace vision {

using namespace ocarina;

struct Description {
protected:
    string_view _type;

public:
    string name;

public:
    Description() = default;
    Description(string_view type, string name)
        : _type(type), name(std::move(name)) {}
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
    void init(const ParameterSet &ps) noexcept override;
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

struct MaterialDesc : public Description {
public:
public:
    VISION_DESC_COMMON(Material)
    void init(const ParameterSet &ps) noexcept override;
};

struct LightDesc : public Description {
    VISION_DESC_COMMON(Light)
    void init(const ParameterSet &ps) noexcept override;
};

struct TextureDesc : public Description {
    VISION_DESC_COMMON(Texture)
    void init(const ParameterSet &ps) noexcept override;
};

struct LightSamplerDesc : public Description {
    VISION_DESC_COMMON(LightSampler)
    void init(const ParameterSet &ps) noexcept override;
};

}// namespace vision