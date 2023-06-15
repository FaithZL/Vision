//
// Created by Zero on 2023/6/15.
//

#include "base/bake.h"

namespace vision {

class SoftwareRasterizer : public Rasterizer {
public:
    explicit SoftwareRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void apply(vision::BakedShape &baked_shape) noexcept override {

    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SoftwareRasterizer)