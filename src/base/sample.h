//
// Created by Zero on 29/10/2022.
//

#pragma once

#include "dsl/common.h"

namespace vision {
using namespace ocarina;

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

struct BSDFEval {
    Float3 f;
    Float pdf{0.f};
    [[nodiscard]] Bool valid() const noexcept {
        return pdf > 0.f;
    }
};

struct LightEval {
    Float3 L;
    Float pdf{0.f};
    [[nodiscard]] Bool valid() const noexcept {
        return pdf > 0.f;
    }
};

struct BSDFSample {
    BSDFEval eval;
    Float3 wi;
    Uchar flags;
    [[nodiscard]] Bool valid() const noexcept {
        return eval.valid();
    }
};

struct LightSample {
    LightEval eval;
    Float3 p_light;
    [[nodiscard]] Bool valid() const noexcept {
        return eval.valid();
    }
};

}// namespace vision