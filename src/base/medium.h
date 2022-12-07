//
// Created by Zero on 29/11/2022.
//

#pragma once

#include "dsl/common.h"
#include "math/optics.h"
#include "node.h"
#include "interaction.h"

namespace vision {
using namespace ocarina;

struct MediumInterface {
public:
    Uchar inside{InvalidUI8};
    Uchar outside{InvalidUI8};

public:
    MediumInterface() = default;
    MediumInterface(Uchar in, Uchar out) : inside(in), outside(out) {}
    explicit MediumInterface(Uchar medium_id) : inside(medium_id), outside(medium_id) {}
    [[nodiscard]] Bool is_transition() const noexcept { return inside != outside; }
    [[nodiscard]] Bool has_inside() const noexcept { return inside != InvalidUI8; }
    [[nodiscard]] Bool has_outside() const noexcept { return outside != InvalidUI8; }
};

template<EPort p = D>
[[nodiscard]] Float phase_HG(Float cos_theta, Float g) {
    Float denom = 1 + sqr(g) + 2 * g * cos_theta;
    return Inv4Pi * (1 - sqr(g)) / (denom * sqrt(denom));
}

class Sampler;

class PhaseFunction {
public:
    virtual void init(Float g) noexcept = 0;
    [[nodiscard]] virtual Bool valid() const noexcept = 0;

    [[nodiscard]] virtual ScatterEval evaluate(Float3 wo, Float3 wi) const noexcept {
        Float val = f(wo, wi);
        return {.f = make_float3(val), .pdf = val};
    }
    [[nodiscard]] virtual PhaseSample sample(Float3 wo, Sampler *sampler) const noexcept = 0;
    [[nodiscard]] virtual Float f(Float3 wo, Float3 wi) const noexcept = 0;
};

class HenyeyGreenstein : public PhaseFunction {
private:
    static constexpr float InvalidG = 10;
    Float _g{InvalidG};

public:
    HenyeyGreenstein() = default;
    explicit HenyeyGreenstein(Float g) : _g(g) {}
    void init(Float g) noexcept override { _g = g; }
    [[nodiscard]] Float f(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] PhaseSample sample(Float3 wo, Sampler *sampler) const noexcept override;
    [[nodiscard]] Bool valid() const noexcept override { return InvalidG != _g; }
};

class Sampler;

class Medium : public Node {
public:
    using Desc = MediumDesc;

public:
    explicit Medium(const MediumDesc &desc) : Node(desc) {}
    ~Medium() override {}
    virtual Float3 Tr(const OCRay &ray, Sampler *sampler) const noexcept = 0;
    virtual Float3 sample(const OCRay &ray, Interaction &it, Sampler *sampler) const noexcept = 0;
};


}// namespace vision