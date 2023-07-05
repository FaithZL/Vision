//
// Created by Zero on 2023/6/14.
//

#include "dilate_filter.h"

namespace vision {

using namespace ocarina;

void DilateFilter::compile() noexcept {
    Kernel kernel = [&](TextureVar src, TextureVar dst) {

    };
    _shader = device().compile(kernel, "dilate filter");
}

ShaderDispatchCommand *DilateFilter::dispatch(const Texture &src,
                                              const Texture &dst) const noexcept {
    return _shader(src, dst).dispatch(src->resolution());
}

}// namespace vision