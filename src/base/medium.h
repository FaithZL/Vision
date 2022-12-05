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
    virtual void init(Float g) noexcept = 0;
    [[nodiscard]] virtual Bool valid() const noexcept = 0;
    [[nodiscard]] virtual Float f(Float3 wo, Float3 wi) const noexcept = 0;

    /**
     * @param wo
     * @param u
     * @return f , wi
     */
    [[nodiscard]] virtual pair<Float, Float3> sample_f(Float3 wo, Float2 u) const noexcept = 0;
};

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

class HenyeyGreenstein : public PhaseFunction {
private:
    static constexpr float InvalidG = 10;
    Float _g{InvalidG};

public:
    HenyeyGreenstein() = default;
    explicit HenyeyGreenstein(Float g) : _g(g) {}
    void init(Float g) noexcept override { _g = g; }
    [[nodiscard]] Float f(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] pair<Float, Float3> sample_f(Float3 wo, Float2 u) const noexcept override;
    [[nodiscard]] Bool valid() const noexcept override { return InvalidG != _g; }
};

class Sampler;

class Medium : public Node {
public:
    using Desc = MediumDesc;

public:
    explicit Medium(const MediumDesc &desc) : Node(desc) {}
    virtual ~Medium() {}
    virtual Float3 Tr(const OCRay &ray, Sampler *sampler) const noexcept = 0;
    virtual pair<Float3, Interaction> sample(const OCRay &ray, Sampler *sampler) const noexcept = 0;
};


}// namespace vision