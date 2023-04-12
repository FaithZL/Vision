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
    explicit AliasTable(ResourceArray &resource_array)
        : _table(resource_array), _func(resource_array) {}
    explicit AliasTable(const WarperDesc &desc)
        : Warper(desc),
          _table(render_pipeline()->resource_array()),
          _func(render_pipeline()->resource_array()) {}
    void prepare() noexcept override;
    void build(vector<float> weights) noexcept override;
    [[nodiscard]] uint size() const noexcept override { return _func.host().size(); }
    [[nodiscard]] uint data_size() const noexcept override {
        return sizeof(_table.index()) + sizeof(_func.index()) + sizeof(size());
    }
    void fill_data(ManagedWrapper<float> &datas) const noexcept override {
        datas.push_back(bit_cast<float>(size()));
        datas.push_back(bit_cast<float>(_func.index()));
        datas.push_back(bit_cast<float>(_table.index()));
    }
    [[nodiscard]] Float func_at(const Uint &i) const noexcept override { return func_at(_func.index(), i); }
    [[nodiscard]] Float PDF(const Uint &i) const noexcept override { return PDF(_func.index(), i); }
    [[nodiscard]] Float PMF(const Uint &i) const noexcept override { return PMF(_func.index(), i); }
    [[nodiscard]] Float func_at(const Uint &buffer_id, const Uint &i) const noexcept override {
        return render_pipeline()->buffer<float>(buffer_id).read(i);
    }
    [[nodiscard]] Float PDF(const Uint &buffer_id, const Uint &i) const noexcept {
        return integral() > 0 ? func_at(buffer_id, i) / integral() : Var(0.f);
    }
    [[nodiscard]] Float PMF(const Uint &buffer_id, const Uint &i) const noexcept {
        return integral() > 0 ? (func_at(buffer_id, i) / (integral() * size())) : Var(0.f);
    }
    [[nodiscard]] Uint sample_discrete(Float u, Float *pmf, Float *u_remapped) const noexcept override {
        return sample_discrete(_func.index(), _table.index(), u, pmf, u_remapped);
    }
    [[nodiscard]] Float sample_continuous(Float u, Float *pdf, Uint *offset) const noexcept override {
        return sample_continuous(_func.index(), _table.index(), u, pdf, offset);
    }
    [[nodiscard]] pair<Uint, Float> offset_u_remapped(Float u, const Uint &entry_id, size_t size) const noexcept;
    [[nodiscard]] Uint sample_discrete(const Uint &func_id, const Uint &entry_id, Float u,
                                       Float *pmf, Float *u_remapped) const noexcept override;
    [[nodiscard]] Float sample_continuous(const Uint &func_id, const Uint &entry_id,
                                          Float u, Float *pdf, Uint *offset) const noexcept override;
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

pair<Uint, Float> AliasTable::offset_u_remapped(Float u, const Uint &entry_id, size_t size) const noexcept {
    u = u * float(size);
    Uint idx = min(cast<uint>(u), uint(size - 1));
    u = min(u - idx, OneMinusEpsilon);
    Var alias_entry = render_pipeline()->buffer<AliasEntry>(entry_id).read(idx);
    idx = select(u < alias_entry.prob, idx, alias_entry.alias);
    Float u_remapped = select(u < alias_entry.prob,
                              min(u / alias_entry.prob, OneMinusEpsilon),
                              min((1 - u) / (1 - alias_entry.prob), OneMinusEpsilon));
    return {idx, u_remapped};
}

Uint AliasTable::sample_discrete(const Uint &func_id, const Uint &entry_id, Float u,
                                 Float *pmf, Float *u_remapped) const noexcept {
    auto [offset, ur] = offset_u_remapped(u, entry_id, size());
    if (pmf) {
        *pmf = PMF(func_id, offset);
    }
    if (u_remapped) {
        *u_remapped = ur;
    }
    return offset;
}

Float AliasTable::sample_continuous(const Uint &func_id, const Uint &entry_id, Float u,
                                    Float *pdf, Uint *offset) const noexcept {
    auto [ofs, u_remapped] = offset_u_remapped(u, entry_id, size());
    Float ret = (ofs + u_remapped) / float(size());
    if (pdf) {
        *pdf = PDF(func_id, ofs);
    }
    if (offset) {
        *offset = ofs;
    }
    return ret;
}

}// namespace vision
