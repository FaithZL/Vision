//
// Created by Zero on 2023/7/6.
//

#include "ocarina/src/windows/gl.h"
#include "ocarina/src/windows/gl_helper.h"
#include "base/bake_utlis.h"

namespace vision {

class OpenGLRasterizer : public Rasterizer {

public:
    explicit OpenGLRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile_shader() noexcept override {

    }

    [[nodiscard]] CommandList apply(vision::BakedShape &baked_shape) noexcept override{
        return {};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OpenGLRasterizer)