//
// Created by Zero on 27/10/2022.
//

#pragma once

#include "dsl/dsl.h"
#include "node.h"
#include "descriptions/node_desc.h"

namespace vision {

class Warper : public Node, public Serializable<> {
public:
    using Desc = WarperDesc;

protected:
    Serial<float> _integral{};

public:
    Warper() = default;
    OC_SERIALIZABLE_FUNC(Serializable<>, _integral)
    explicit Warper(const WarperDesc &desc) : Node(desc) {}
    virtual void build(vector<float> weights) noexcept = 0;
    [[nodiscard]] virtual Uint size() const noexcept = 0;
    [[nodiscard]] virtual Serial<float> integral() const noexcept { return _integral; }
    [[nodiscard]] virtual Float func_at(const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float PDF(const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float PMF(const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float combine(const Uint &index, const Float &u) const noexcept = 0;
    [[nodiscard]] virtual Uint sample_discrete(Float u, Float *pmf, Float *u_remapped) const noexcept = 0;
    [[nodiscard]] virtual Float sample_continuous(Float u, Float *pdf, Uint *offset) const noexcept = 0;
};

class Warper2D : public Node, public Serializable<> {
public:
    using Desc = WarperDesc;

public:
    Warper2D() = default;
    explicit Warper2D(const WarperDesc &desc) : Node(desc) {}
    virtual void build(vector<float> weights, uint2 res) noexcept = 0;
    [[nodiscard]] virtual Float func_at(Uint2 coord) const noexcept = 0;
    [[nodiscard]] virtual Float PDF(Float2 p) const noexcept = 0;
    [[nodiscard]] virtual Serial<float> integral() const noexcept = 0;
    [[nodiscard]] virtual Float2 sample_continuous(Float2 u, Float *pdf, Uint2 *coord) const noexcept = 0;
};

}// namespace vision