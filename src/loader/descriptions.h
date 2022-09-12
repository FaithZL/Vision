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

protected:
    explicit Description(string_view type, const string &name)
        : _type(type), name(name) {}
};

struct SamplerDesc : public Description {
public:
    uint spp{};

public:
    explicit SamplerDesc(const string &name) : Description("Sampler", name) {}
};

struct SensorDesc : public Description {

};

struct FilterDesc : public Description {
};

struct IntegratorDesc : public Description {
};

struct MaterialDesc : public Description {
};

struct LightDesc : public Description {
};

struct TextureDesc : public Description {
};

struct LightSamplerDesc : public Description {
};

}// namespace vision