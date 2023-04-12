//
// Created by Zero on 27/10/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"
#include "descriptions/node_desc.h"

namespace vision {

class Warper : public Node, public PolymorphicElement<float> {
public:
    using Desc = WarperDesc;

protected:
    float _integral{};

public:
    Warper() = default;
    explicit Warper(const WarperDesc &desc) : Node(desc) {}
    virtual void build(vector<float> weights) noexcept = 0;
    [[nodiscard]] virtual uint size() const noexcept = 0;
    [[nodiscard]] virtual float integral() const noexcept { return _integral; }
    [[nodiscard]] virtual Float func_at(const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float PDF(const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float PMF(const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float func_at(const Uint &buffer_id, const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float PDF(const Uint &buffer_id, const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float PMF(const Uint &buffer_id, const Uint &i) const noexcept = 0;
    /**
     * @param u uniform
     * @return offset, PMF, u_remapped
     */
    [[nodiscard]] virtual tuple<Uint, Float, Float> sample_discrete(Float u) const noexcept = 0;
    /**
     * @param u uniform
     * @return ret, PDF, offset
     */
    [[nodiscard]] virtual tuple<Float, Float, Uint> sample_continuous(Float u) const noexcept = 0;

    [[nodiscard]] virtual Uint sample_discrete(Float u, Float *pmf, Float *u_remapped) const noexcept = 0;
    [[nodiscard]] virtual Float sample_continuous(Float u, Float *pdf, Uint *offset) const noexcept = 0;
    [[nodiscard]] virtual Uint sample_discrete(const Uint &func_id, const Uint &entry_id, Float u,
                                               Float *pmf, Float *u_remapped) const noexcept = 0;
    [[nodiscard]] virtual Float sample_continuous(const Uint &func_id, const Uint &entry_id, Float u,
                                                  Float *pdf, Uint *offset) const noexcept = 0;
};

class Warper2D : public Node {
public:
    using Desc = WarperDesc;

public:
    Warper2D() = default;
    explicit Warper2D(const WarperDesc &desc) : Node(desc) {}
    virtual void build(vector<float> weights, uint2 res) noexcept = 0;
    [[nodiscard]] virtual Float func_at(Uint2 coord) const noexcept = 0;
    [[nodiscard]] virtual Float PDF(Float2 p) const noexcept = 0;
    [[nodiscard]] virtual float integral() const noexcept = 0;
    [[nodiscard]] virtual tuple<Float2, Float, Uint2> sample_continuous(Float2 u) const noexcept = 0;
};

}// namespace vision