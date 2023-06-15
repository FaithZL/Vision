//
// Created by Zero on 2023/6/15.
//

#include "base/rasterizer.h"

namespace vision {

class SoftwareRasterizer : public Rasterizer {
public:
    explicit SoftwareRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SoftwareRasterizer)