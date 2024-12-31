//
// Created by Zero on 30/11/2022.
//

#pragma once

#include "rhi/common.h"
#include "base/mgr/pipeline.h"
#include "base/warper.h"

namespace vision {
struct AliasEntry {
    float prob;
    uint alias;
};
}// namespace vision

OC_STRUCT(vision, AliasEntry, prob, alias){};

namespace vision {

class AliasTable2D;

class AliasTable : public Warper {
private:
    RegistrableManaged<AliasEntry> table_;
    RegistrableManaged<float> func_;
    friend class AliasTable2D;

public:
    explicit AliasTable(BindlessArray &bindless_array)
        : table_(bindless_array), func_(bindless_array) {}
    explicit AliasTable(const WarperDesc &desc)
        : Warper(desc),
          table_(pipeline()->bindless_array()),
          func_(pipeline()->bindless_array()) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    OC_ENCODABLE_FUNC(Warper, table_, func_)
    void prepare() noexcept override;
    void clear() noexcept override;
    void upload_immediately() noexcept override;
    void build(vector<float> weights) noexcept override;
    void allocate(uint num) noexcept override;
    [[nodiscard]] Uint size() const noexcept override { return *func_.length(); }
    [[nodiscard]] Float PDF(const Uint &i) const noexcept override {
        return select(*integral() > 0, func_at(i) / *integral(), 0.f);
    }
    [[nodiscard]] Float func_at(const Uint &i) const noexcept override {
        return func_.read(i);
    }
    [[nodiscard]] Float PMF(const Uint &i) const noexcept override {
        return select(*integral() > 0, func_at(i) / (*integral() * size()), 0.f);
    }
    [[nodiscard]] Float combine(const Uint &index, const Float &u) const noexcept override;
    [[nodiscard]] pair<Uint, Float> offset_u_remapped(Float u, const Uint &size) const noexcept;
    [[nodiscard]] Uint sample_discrete(Float u, Float *pmf, Float *u_remapped) const noexcept override;
    [[nodiscard]] Float sample_continuous(Float u, Float *pdf, Uint *offset) const noexcept override;
};

void AliasTable::clear() noexcept {
    table_.clear();
    func_.clear();
}

void AliasTable::allocate(uint num) noexcept {
    table_.device_buffer() = pipeline()->device().create_buffer<AliasEntry>(num, "AliasTable::table_");
    func_.device_buffer() = pipeline()->device().create_buffer<float>(num, "AliasTable::func_");
    table_.register_self();
    func_.register_self();
}

void AliasTable::upload_immediately() noexcept {
    table_.upload_immediately();
    func_.upload_immediately();
}

void AliasTable::prepare() noexcept {
    table_.reset_device_buffer_immediately(device(), "AliasTable::table_");
    func_.reset_device_buffer_immediately(device(), "AliasTable::func_");
    table_.upload_immediately();
    func_.upload_immediately();

    table_.register_self();
    func_.register_self();
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

    integral_ = sum / weights.size();
    func_.set_host(std::move(weights));
    table_.set_host(std::move(table));
}

namespace detail {

[[nodiscard]] Uint offset(const Uint &buffer_offset, Float u, const Pipeline *rp,
                          const Uint &entry_id, const Uint &size, Float *u_remapped) noexcept {
    u = u * cast<float>(size);
    Uint idx = min(cast<uint>(u), size - 1);
    u = min(u - idx, OneMinusEpsilon);
    Var alias_entry = rp->buffer_var<AliasEntry>(entry_id).read(buffer_offset + idx);
    idx = select(u < alias_entry.prob, idx, alias_entry.alias);
    if (u_remapped) {
        *u_remapped = select(u < alias_entry.prob,
                             min(u / alias_entry.prob, OneMinusEpsilon),
                             min((1 - u) / (1 - alias_entry.prob), OneMinusEpsilon));
    }
    return idx;
}

}// namespace detail

Float AliasTable::combine(const Uint &index, const Float &u) const noexcept {
    Float ret = (index + u) / cast<float>(size());
    return ret;
}

pair<Uint, Float> AliasTable::offset_u_remapped(Float u, const Uint &size) const noexcept {
    u = u * size;
    Uint idx = min(cast<uint>(u), size - 1);
    u = min(u - idx, OneMinusEpsilon);
    Var alias_entry = table_.read(idx);
    idx = select(u < alias_entry.prob, idx, alias_entry.alias);
    Float u_remapped = select(u < alias_entry.prob,
                              min(u / alias_entry.prob, OneMinusEpsilon),
                              min((1 - u) / (1 - alias_entry.prob), OneMinusEpsilon));
    return {idx, u_remapped};
}

Uint AliasTable::sample_discrete(Float u, Float *pmf, Float *u_remapped) const noexcept {
    auto [offset, ur] = offset_u_remapped(u, size());
    if (pmf) {
        *pmf = PMF(offset);
    }
    if (u_remapped) {
        *u_remapped = ur;
    }
    return offset;
}

Float AliasTable::sample_continuous(Float u, Float *pdf, Uint *offset) const noexcept {
    auto [ofs, u_remapped] = offset_u_remapped(u, size());
    Float ret = (ofs + u_remapped) / cast<float>(size());
    if (pdf) {
        *pdf = PDF(ofs);
    }
    if (offset) {
        *offset = ofs;
    }
    return ret;
}

}// namespace vision
