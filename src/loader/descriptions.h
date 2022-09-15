//
// Created by Zero on 10/09/2022.
//

#pragma once

#include "core/stl.h"
#include "core/basic_types.h"

namespace vision {

using namespace ocarina;

struct Description {
protected:
    string_view _type;

public:
    string name;

public:
    Description() = default;

    explicit Description(string_view type, const string &name)
        : _type(type), name(name) {}

    virtual void init(const DataWrap &data) noexcept = 0;
};

struct TransformDesc : public Description {
public:
    float3 t;
    float4 r;
    float3 s;

    float4x4 mat;

    float3 position;
    float yaw{};
    float pitch{};

    float3 up{};
    float3 target_pos{};

public:
    void init(const DataWrap &data) noexcept override {

    }
};

struct ShapeDesc : public Description {
public:
    void init(const DataWrap &data) noexcept override {

    }
};

struct SamplerDesc : public Description {
public:
    uint spp{};

public:
    SamplerDesc() = default;
    explicit SamplerDesc(const string &name) : Description("Sampler", name) {}

    void init(const DataWrap &data) noexcept override {

    }
};

struct SensorDesc : public Description {
    void init(const DataWrap &data) noexcept override {

    }
};

struct FilterDesc : public Description {
    void init(const DataWrap &data) noexcept override {

    }
};

struct IntegratorDesc : public Description {
public:
    uint max_depth = 10;
    uint min_depth = 5;
    float rr_threshold = 1;

public:
    void init(const DataWrap &data) noexcept override {

    }
};

struct MaterialDesc : public Description {
    void init(const DataWrap &data) noexcept override {

    }
};

struct LightDesc : public Description {
    void init(const DataWrap &data) noexcept override {

    }
};

struct TextureDesc : public Description {
    void init(const DataWrap &data) noexcept override {

    }
};

struct LightSamplerDesc : public Description {
    void init(const DataWrap &data) noexcept override {

    }
};

}// namespace vision