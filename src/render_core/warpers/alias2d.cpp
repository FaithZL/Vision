//
// Created by Zero on 30/11/2022.
//

#include "alias.h"

namespace vision {

class AliasTable2D : public Warper2D {
private:
    AliasTable _marginal;
    RegistrableManaged<AliasEntry> _conditional_v_tables;
    RegistrableManaged<float> _conditional_v_weights;
    uint2 _resolution;

public:
    explicit AliasTable2D(const WarperDesc &desc)
        : Warper2D(desc),
          _marginal(render_pipeline()->resource_array()),
          _conditional_v_tables(render_pipeline()->resource_array()),
          _conditional_v_weights(render_pipeline()->resource_array()) {
        _marginal._scene = desc.scene;
    }
    OC_SERIALIZABLE_FUNC(_marginal, _conditional_v_tables, _conditional_v_weights)
    void build(vector<float> weights, uint2 res) noexcept override {
        // build conditional_v
        vector<AliasTable> conditional_v;
        conditional_v.reserve(res.y);
        auto iter = weights.begin();
        for (int v = 0; v < res.y; ++v) {
            vector<float> func_v;
            func_v.insert(func_v.end(), iter, iter + res.x);
            iter += res.x;
            AliasTable alias_table(render_pipeline()->resource_array());
            alias_table._scene = _scene;
            alias_table.build(ocarina::move(func_v));
            conditional_v.push_back(ocarina::move(alias_table));
        }

        // build marginal
        vector<float> marginal_func;
        marginal_func.reserve(res.y);
        for (int v = 0; v < res.y; ++v) {
            marginal_func.push_back(conditional_v[v].integral());
        }
        _marginal.build(ocarina::move(marginal_func));

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
    void prepare() noexcept override {
        _marginal.prepare();
        _conditional_v_tables.reset_device_buffer(device());
        _conditional_v_weights.reset_device_buffer(device());
        _conditional_v_tables.upload_immediately();
        _conditional_v_weights.upload_immediately();

        _conditional_v_weights.register_self();
        _conditional_v_tables.register_self();
    }
    [[nodiscard]] Float func_at(Uint2 coord) const noexcept override {
        Uint idx = coord.y * _resolution.x + coord.x;
        return _conditional_v_weights.read(idx);
    }
    [[nodiscard]] Float PDF(Float2 p) const noexcept override {
        Uint iu = clamp(cast<uint>(p.x * _resolution.x), 0u, _resolution.x - 1);
        Uint iv = clamp(cast<uint>(p.y * _resolution.y), 0u, _resolution.y - 1);
        return integral() > 0 ? func_at(make_uint2(iu, iv)) / integral() : Var(0.f);
    }
    [[nodiscard]] float integral() const noexcept override {
        return _marginal.integral();
    }
    [[nodiscard]] Float2 sample_continuous(Float2 u, Float *pdf, Uint2 *coord) const noexcept override {
        // sample v
        Float pdf_v;
        Uint iv;
        Float fv = _marginal.sample_continuous(u.y, &pdf_v, &iv);

        // sample u
        Uint buffer_offset = _resolution.x * iv;
        Float u_remapped;
        Uint iu = detail::offset(buffer_offset, u.x, render_pipeline(),
                                 _conditional_v_tables.index().hv(), _resolution.x, &u_remapped);

        Float fu = (iu + u_remapped) / _resolution.x;
        Float integral_u = _marginal._func.read(iv);
        Float func_u = _conditional_v_weights.read(buffer_offset + iu);
        Float pdf_u = select(integral_u > 0, func_u / integral_u, 0.f);
        if (pdf) {
            *pdf = pdf_u * pdf_v;
        }
        if (coord) {
            *coord = make_uint2(iu, iv);
        }
        return make_float2(fu, fv);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AliasTable2D)