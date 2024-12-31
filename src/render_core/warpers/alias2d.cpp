//
// Created by Zero on 30/11/2022.
//

#include "alias.h"

namespace vision {

class AliasTable2D : public Warper2D {
private:
    AliasTable marginal_;
    RegistrableManaged<AliasEntry> conditional_v_tables_;
    RegistrableManaged<float> conditional_v_weights_;
    EncodedData<uint2> resolution_;

public:
    explicit AliasTable2D(const WarperDesc &desc)
        : Warper2D(desc),
          marginal_(pipeline()->bindless_array()),
          conditional_v_tables_(pipeline()->bindless_array()),
          conditional_v_weights_(pipeline()->bindless_array()) {
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    OC_ENCODABLE_FUNC(Warper2D, marginal_, conditional_v_tables_,
                      conditional_v_weights_, resolution_)
    [[nodiscard]] Uint2 resolution() const noexcept { return *resolution_; }
    void clear() noexcept override {
        marginal_.clear();
        conditional_v_tables_.clear();
        conditional_v_weights_.clear();
    }
    void build(vector<float> weights, uint2 res) noexcept override {
        clear();
        // build conditional_v
        vector<AliasTable> conditional_v;
        conditional_v.reserve(res.y);
        auto iter = weights.begin();
        for (int v = 0; v < res.y; ++v) {
            vector<float> func_v;
            func_v.insert(func_v.end(), iter, iter + res.x);
            iter += res.x;
            AliasTable alias_table(pipeline()->bindless_array());
            alias_table.build(ocarina::move(func_v));
            conditional_v.push_back(ocarina::move(alias_table));
        }

        // build marginal
        vector<float> marginal_func;
        marginal_func.reserve(res.y);
        for (int v = 0; v < res.y; ++v) {
            marginal_func.push_back(conditional_v[v].integral().hv());
        }
        marginal_.build(ocarina::move(marginal_func));

        // flatten conditionals table and weight
        conditional_v_tables_.reserve(res.x * res.y);
        conditional_v_weights_.reserve(res.x * res.y);

        for (auto &alias_table : conditional_v) {
            conditional_v_tables_.insert(conditional_v_tables_.cend(),
                                         alias_table.table_.begin(),
                                         alias_table.table_.end());
            conditional_v_weights_.insert(conditional_v_weights_.cend(),
                                          alias_table.func_.begin(),
                                          alias_table.func_.end());
        }
        resolution_ = res;
    }
    void upload_immediately() noexcept override {
        marginal_.upload_immediately();
        conditional_v_tables_.upload_immediately();
        conditional_v_weights_.upload_immediately();
    }
    void allocate(uint2 res) noexcept override {
        marginal_.allocate(res.y);
        uint num = res.x * res.y;
        conditional_v_tables_.device_buffer() = pipeline()->device().create_buffer<AliasEntry>(num, "AliasTable2D::conditional_v_tables_");
        conditional_v_weights_.device_buffer() = pipeline()->device().create_buffer<float>(num, "AliasTable2D::conditional_v_weights_");
        conditional_v_tables_.register_self();
        conditional_v_tables_.register_self();
    }
    void prepare() noexcept override {
        marginal_.prepare();
        conditional_v_tables_.reset_device_buffer_immediately(device(),
                                                              "AliasTable2D::conditional_v_tables_");
        conditional_v_weights_.reset_device_buffer_immediately(device(),
                                                               "AliasTable2D::conditional_v_weights_");
        conditional_v_tables_.upload_immediately();
        conditional_v_weights_.upload_immediately();

        conditional_v_weights_.register_self();
        conditional_v_tables_.register_self();
    }
    [[nodiscard]] Float func_at(Uint2 coord) const noexcept override {
        Uint idx = coord.y * resolution().x + coord.x;
        return conditional_v_weights_.read(idx);
    }
    [[nodiscard]] Float PDF(Float2 p) const noexcept override {
        Uint iu = clamp(cast<uint>(p.x * resolution().x), 0u, resolution().x - 1);
        Uint iv = clamp(cast<uint>(p.y * resolution().y), 0u, resolution().y - 1);
        return select(*integral() > 0, func_at(make_uint2(iu, iv)) / *integral(), 0.f);
    }
    [[nodiscard]] EncodedData<float> integral() const noexcept override {
        return marginal_.integral();
    }
    [[nodiscard]] Float2 sample_continuous(Float2 u, Float *pdf, Uint2 *coord) const noexcept override {
        // sample v
        Float pdf_v;
        Uint iv;
        Float fv = marginal_.sample_continuous(u.y, std::addressof(pdf_v), std::addressof(iv));

        // sample u
        Uint buffer_offset = resolution().x * iv;
        Float u_remapped;
        Uint iu = detail::offset(buffer_offset, u.x, pipeline(),
                                 conditional_v_tables_.index().hv(), resolution().x, &u_remapped);

        Float fu = (iu + u_remapped) / resolution().x;
        Float integral_u = marginal_.func_.read(iv);
        Float func_u = conditional_v_weights_.read(buffer_offset + iu);
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