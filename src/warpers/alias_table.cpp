//
// Created by Zero on 27/10/2022.
//

#include "base/warper.h"
#include "rhi/common.h"
#include "base/render_pipeline.h"

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
    void prepare(RenderPipeline *rp) noexcept override {
        _table.reset_device_buffer(rp->device());
        _func.reset_device_buffer(rp->device());
        _table.upload_immediately();
        _func.upload_immediately();
    }
    void build(vector<float> weights) noexcept override {
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
    [[nodiscard]] uint size() const noexcept override { return _func.host().size(); }
    [[nodiscard]] Float func_at(const Uint &i) const noexcept override { return _func.read(i); }
    [[nodiscard]] Float PDF(const Uint &i) const noexcept override {
        return integral() > 0 ? func_at(i) / integral() : Var(0.f);
    }
    [[nodiscard]] Float PMF(const Uint &i) const noexcept override {
        return integral() > 0 ? (func_at(i) / (integral() * size())) : Var(0.f);
    }
    [[nodiscard]] tuple<Uint, Float> offset_u_remapped(Float u) const noexcept {
        u = u * float(size());
        Uint offset = min(cast<uint>(u), uint(size() - 1));
        u = min(u - offset, OneMinusEpsilon);
        Var alias_entry = _table.read(offset);
        offset = select(u < alias_entry.prob, offset, alias_entry.alias);
        Float u_remapped = select(u < alias_entry.prob,
                                  min(u / alias_entry.prob, OneMinusEpsilon),
                                  min((1 - u) / (1 - alias_entry.prob), OneMinusEpsilon));
        return {offset, u_remapped};
    }
    [[nodiscard]] tuple<Float, Float, Uint> sample_continuous(Float u) const noexcept override {
        auto [offset, u_remapped] = offset_u_remapped(u);
        Float ret = (offset + u_remapped) / float(size());
        return {ret, PDF(offset), offset};
    }
    [[nodiscard]] tuple<Uint, Float, Float> sample_discrete(Float u) const noexcept override {
        auto [offset, u_remapped] = offset_u_remapped(u);
        return {offset, PMF(offset), u_remapped};
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AliasTable)