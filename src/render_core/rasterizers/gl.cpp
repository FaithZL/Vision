//
// Created by Zero on 2023/7/10.
//

#include "base/bake_utlis.h"

namespace vision {

class OpenGLRasterizer : public Rasterizer {
public:
    explicit OpenGLRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile() noexcept override {
    }

    void apply(vision::BakedShape &baked_shape) noexcept override {
        auto &stream = pipeline()->stream();
    }
};

}// namespace vision