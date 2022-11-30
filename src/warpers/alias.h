//
// Created by Zero on 30/11/2022.
//

#pragma once

#include "rhi/common.h"
#include "base/render_pipeline.h"
#include "base/warper.h"

namespace vision {
struct AliasEntry {
    float prob;
    uint alias;
};
}// namespace vision

OC_STRUCT(vision::AliasEntry, prob, alias){};

namespace vision {
class AliasTable : public Warper {
private:
    Managed<AliasEntry> _table;
    Managed<float> _func;

public:
    explicit AliasTable(const WarperDesc &desc) : Warper(desc) {}
    void prepare(RenderPipeline *rp) noexcept override;
    void build(vector<float> weights) noexcept override;
    [[nodiscard]] uint size() const noexcept override { return _func.host().size(); }
    [[nodiscard]] Float func_at(const Uint &i) const noexcept override { return _func.read(i); }
    [[nodiscard]] Float PDF(const Uint &i) const noexcept override;
    [[nodiscard]] Float PMF(const Uint &i) const noexcept override;
    [[nodiscard]] tuple<Uint, Float> offset_u_remapped(Float u) const noexcept;
    [[nodiscard]] tuple<Float, Float, Uint> sample_continuous(Float u) const noexcept override;
    [[nodiscard]] tuple<Uint, Float, Float> sample_discrete(Float u) const noexcept override;
};
}// namespace vision
