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
template<EPort p = D>
[[nodiscard]] Float phase_HG(Float cos_theta, Float g) {
    Float denom = 1 + sqr(g) + 2 * g * cos_theta;
    return Inv4Pi * (1 - sqr(g)) / (denom * sqrt(denom));
}

class PhaseFunction {
public:
    [[nodiscard]] virtual Float f(Float3 wo, Float3 wi) const noexcept = 0;
    [[nodiscard]] virtual pair<Float, Float3> sample_f(Float3 wo, Float2 u) const noexcept = 0;
};

class HenyeyGreenstein : public PhaseFunction {
private:
    Float _g;

public:
    explicit HenyeyGreenstein(Float g) : _g(g) {}
    [[nodiscard]] Float f(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] pair<Float, Float3> sample_f(Float3 wo, Float2 u) const noexcept override;
};

class Sampler;

struct MediumInterface {
public:
    Uint inside{InvalidUI32};
    Uint outside{InvalidUI32};

public:
    MediumInterface() = default;
    MediumInterface(Uint in, Uint out) : inside(in), outside(out) {}
    explicit MediumInterface(Uint medium_id): inside(medium_id), outside(medium_id) {}
    [[nodiscard]] Bool is_transition() const noexcept { return inside != outside; }
};

class Medium : public Node {
public:
    using Desc = MediumDesc;

public:
    explicit Medium(const MediumDesc &desc) : Node(desc) {}
    virtual ~Medium() {}
    virtual Float3 tr(const OCRay &ray, Sampler &sampler) const noexcept = 0;
    virtual pair<Float3, Interaction> sample(const OCRay &ray, Sampler &sampler) const noexcept = 0;
};


}// namespace vision