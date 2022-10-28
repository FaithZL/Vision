//
// Created by Zero on 27/10/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"
#include "descriptions/node_desc.h"

namespace vision {

class Distribution : public Node {
public:
    using Desc = DistributionDesc;

protected:
    float _integral{};

public:
    explicit Distribution(const DistributionDesc &desc) : Node(desc) {}
    virtual void build(vector<float> weights) noexcept = 0;
    [[nodiscard]] virtual uint size() const noexcept = 0;
    [[nodiscard]] virtual float integral() const noexcept { return _integral; }
    [[nodiscard]] virtual Float func_at(const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float PDF(const Uint &i) const noexcept = 0;
    [[nodiscard]] virtual Float PMF(const Uint &i) const noexcept = 0;
    /**
     * @param u uniform
     * @return offset, PMF, u_remapped
     */
    [[nodiscard]] virtual tuple<Int, Float, Float> sample_discrete(Float u) const noexcept = 0;
    /**
     * @param u uniform
     * @return ret, PDF, offset
     */
    [[nodiscard]] virtual tuple<Float, Float, Int> sample_continuous(Float u) const noexcept = 0;
};

}// namespace vision