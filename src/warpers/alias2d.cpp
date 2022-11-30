//
// Created by Zero on 30/11/2022.
//

#include "alias.h"

namespace vision {

class AliasTable2D : public Warper2D {
private:
    AliasTable _impl;
    uint2 _resolution;

public:
    explicit AliasTable2D(const WarperDesc &desc) : Warper2D(desc) {
    }
    void prepare(RenderPipeline *rp) noexcept override {

    }
    void build(RenderPipeline *rp, vector<float> weights, uint2 res) noexcept override {

    }
    [[nodiscard]] Float func_at(Uint2 coord) const noexcept override {
        return 0;
    }
    [[nodiscard]] Float PDF(Float2 p) const noexcept override {
        return 0;
    }
    [[nodiscard]] float integral() const noexcept override {
        return 0;
    }
    [[nodiscard]] tuple<Float2, Float, Uint2> sample_continuous(Float2 u) const noexcept override {
        return {};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AliasTable2D)