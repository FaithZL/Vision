//
// Created by Zero on 30/11/2022.
//

#pragma once

#include "rhi/common.h"
#include "base/mgr/render_pipeline.h"
#include "base/warper.h"

namespace vision {
struct AliasEntry {
    float prob;
    uint alias;
};
}// namespace vision

OC_STRUCT(vision::AliasEntry, prob, alias){};

namespace vision {

class AliasTable2D;

class AliasTable : public Warper {
private:
    ManagedWrapper<AliasEntry> _table;
    ManagedWrapper<float> _func;
    friend class AliasTable2D;

public:
    explicit AliasTable(ResourceArray &bindless_array)
        : _table(bindless_array), _func(bindless_array) {}
    explicit AliasTable(const WarperDesc &desc)
        : Warper(desc),
          _table(render_pipeline()->resource_array()),
          _func(render_pipeline()->resource_array()) {}
    void prepare() noexcept override;
    void build(vector<float> weights) noexcept override;
    [[nodiscard]] uint size() const noexcept override { return _func.host().size(); }
    [[nodiscard]] Float func_at(const Uint &i) const noexcept override { return _func.read(i); }
    [[nodiscard]] Float PDF(const Uint &i) const noexcept override;
    [[nodiscard]] Float PMF(const Uint &i) const noexcept override;
    [[nodiscard]] tuple<Uint, Float> offset_u_remapped(Float u) const noexcept;
    [[nodiscard]] tuple<Float, Float, Uint> sample_continuous(Float u) const noexcept override;
    [[nodiscard]] tuple<Uint, Float, Float> sample_discrete(Float u) const noexcept override;
};

void AliasTable::prepare() noexcept {
    _table.reset_device_buffer(device());
    _func.reset_device_buffer(device());
    _table.upload_immediately();
    _func.upload_immediately();

    _table.register_self();
    _func.register_self();
}
void AliasTable::build(vector<float> weights) noexcept {
    double sum = std::reduce(weights.cbegin(), weights.cend(), 0.0);
    double ratio = static_cast<double>(weights.size()) / sum;
    static thread_local vector<uint> over;
    static thread_local vector<uint> under;
    over.clear();
    under.clear();
    over.reserve(next_pow2(weights.size()));
    under.reserve(next_pow2(weights.size()));

    vector<AliasEntry> table(weights.size());
    for (auto i = 0u; i < weights.size(); i++) {
        auto p = static_cast<float>(weights[i] * ratio);
        table[i] = {p, i};
        (p > 1.0f ? over : under).emplace_back(i);
    }

    while (!over.empty() && !under.empty()) {
        auto o = over.back();
        auto u = under.back();
        over.pop_back();
        under.pop_back();
        table[o].prob -= 1.0f - table[u].prob;
        table[u].alias = o;
        if (table[o].prob > 1.0f) {
            over.push_back(o);
        } else if (table[o].prob < 1.0f) {
            under.push_back(o);
        }
    }
    for (auto i : over) { table[i] = {1.0f, i}; }
    for (auto i : under) { table[i] = {1.0f, i}; }

    _integral = sum / weights.size();
    _func.set_host(std::move(weights));
    _table.set_host(std::move(table));
}
[[nodiscard]] Float AliasTable::PDF(const Uint &i) const noexcept {
    return integral() > 0 ? func_at(i) / integral() : Var(0.f);
}
[[nodiscard]] Float AliasTable::PMF(const Uint &i) const noexcept {
    return integral() > 0 ? (func_at(i) / (integral() * size())) : Var(0.f);
}

namespace detail {

[[nodiscard]] tuple<Uint, Float> offset_u_remapped(Uint buffer_offset, Float u,
                                                   const ManagedWrapper<AliasEntry> &table, size_t size) noexcept {
    u = u * float(size);
    Uint idx = min(cast<uint>(u), uint(size - 1));
    u = min(u - idx, OneMinusEpsilon);
    Var alias_entry = table.read(buffer_offset + idx);
    idx = select(u < alias_entry.prob, idx, alias_entry.alias);
    Float u_remapped = select(u < alias_entry.prob,
                              min(u / alias_entry.prob, OneMinusEpsilon),
                              min((1 - u) / (1 - alias_entry.prob), OneMinusEpsilon));
    return {idx, u_remapped};
}

}// namespace detail

[[nodiscard]] tuple<Uint, Float> AliasTable::offset_u_remapped(Float u) const noexcept {
    return detail::offset_u_remapped(0, u, _table, size());
}
[[nodiscard]] tuple<Float, Float, Uint> AliasTable::sample_continuous(Float u) const noexcept {
    auto [offset, u_remapped] = offset_u_remapped(u);
    Float ret = (offset + u_remapped) / float(size());
    return {ret, PDF(offset), offset};
}
[[nodiscard]] tuple<Uint, Float, Float> AliasTable::sample_discrete(Float u) const noexcept {
    auto [offset, u_remapped] = offset_u_remapped(u);
    return {offset, PMF(offset), u_remapped};
}

}// namespace vision
