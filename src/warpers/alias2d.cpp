//
// Created by Zero on 30/11/2022.
//

#include "alias.h"

namespace vision {

class AliasTable2D : public Warper2D {
private:
    AliasTable _marginal;
    Managed<AliasEntry> _conditional_v_tables;
    Managed<float> _conditional_v_weights;
    uint2 _resolution;

public:
    explicit AliasTable2D(const WarperDesc &desc) : Warper2D(desc) {}
    void build(RenderPipeline *rp, vector<float> weights, uint2 res) noexcept override {
        // build conditional_v
        vector<AliasTable> conditional_v;
        conditional_v.reserve(res.y);
        auto iter = weights.begin();
        for (int v = 0; v < res.y; ++v) {
            vector<float> func_v;
            func_v.insert(func_v.end(), iter, iter + res.x);
            iter += res.x;
            AliasTable alias_table;
            alias_table.build(move(func_v));
            conditional_v.push_back(move(alias_table));
        }

        // build marginal
        vector<float> marginal_func;
        marginal_func.reserve(res.y);
        for (int v = 0; v < res.y; ++v) {
            marginal_func.push_back(conditional_v[v].integral());
        }
        _marginal.build(move(marginal_func));

        // flatten conditionals table and weight
        _conditional_v_tables.reserve(res.x * res.y);
        _conditional_v_weights.reserve(res.x * res.y);

        for (auto &alias_table : conditional_v) {
            _conditional_v_tables.insert(_conditional_v_tables.cend(),
                                         alias_table._table.begin(),
                                         alias_table._table.end());
            _conditional_v_weights.insert(_conditional_v_weights.cend(),
                                          alias_table._func.begin(),
                                          alias_table._func.end());
        }
        _resolution = res;
    }
    void prepare(RenderPipeline *rp) noexcept override {
        _marginal.prepare(rp);
        _conditional_v_tables.reset_device_buffer(rp->device());
        _conditional_v_weights.reset_device_buffer(rp->device());
        _conditional_v_tables.upload_immediately();
        _conditional_v_weights.upload_immediately();
    }
    [[nodiscard]] Float func_at(Uint2 coord) const noexcept override {
        Uint idx = coord.x * _resolution.y + coord.x;
        return _conditional_v_weights.read(idx);
    }
    [[nodiscard]] Float PDF(Float2 p) const noexcept override {
        Uint iu = clamp(cast<uint>(p.x * _resolution.x), 0u, _resolution.x - 1);
        Uint iv = clamp(cast<uint>(p.y * _resolution.y), 0u, _resolution.y - 1);
        return func_at(make_uint2(iu, iv)) / integral();
    }
    [[nodiscard]] float integral() const noexcept override {
        return _marginal.integral();
    }
    [[nodiscard]] tuple<Float2, Float, Uint2> sample_continuous(Float2 u) const noexcept override {
        auto [d1, pdf1, iv] = _marginal.sample_continuous(u.y);
        Uint offset = _resolution.x * iv;
        return {};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AliasTable2D)