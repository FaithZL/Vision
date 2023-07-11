//
// Created by Zero on 2023/7/11.
//


#include "base/bake_utlis.h"

namespace vision {

class GPURasterizer : public Rasterizer {
private:

public:
    explicit GPURasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile() noexcept override {
    }

    void apply(vision::BakedShape &baked_shape) noexcept override {
        auto &stream = pipeline()->stream();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GPURasterizer)