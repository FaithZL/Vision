//
// Created by Zero on 11/10/2022.
//

#pragma once

#include "dsl/common.h"

namespace vision {

using namespace ocarina;

struct SensorSample {
    Float2 p_film;
    Float2 p_lens;
    Float time;
    Float filter_weight{1.f};
};

struct RaySample {
    OCRay ray;
    Float weight;
};

struct BxDFFlag {
    static constexpr uchar Unset = 1;
    static constexpr uchar Reflection = 1 << 1;
    static constexpr uchar Transmission = 1 << 2;
    static constexpr uchar Diffuse = 1 << 3;
    static constexpr uchar Glossy = 1 << 4;
    static constexpr uchar Specular = 1 << 5;
    static constexpr uchar NearSpec = 1 << 6;
    // Composite _BxDFFlags_ definitions
    static constexpr uchar DiffRefl = Diffuse | Reflection;
    static constexpr uchar DiffTrans = Diffuse | Transmission;
    static constexpr uchar GlossyRefl = Glossy | Reflection;
    static constexpr uchar GlossyTrans = Glossy | Transmission;
    static constexpr uchar SpecRefl = Specular | Reflection;
    static constexpr uchar SpecTrans = Specular | Transmission;
    static constexpr uchar All = Diffuse | Glossy | Specular | Reflection | Transmission | NearSpec;
};

struct BSDFSample {
    Float3 val;
    Float pdf;
    Float3 wi;
    Uchar flags;
};

}// namespace vision