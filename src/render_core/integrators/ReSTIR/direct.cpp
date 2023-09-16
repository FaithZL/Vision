//
// Created by Zero on 2023/9/15.
//

#include "direct.h"
#include "base/mgr/pipeline.h"

namespace vision {

void ReSTIRDI::compile_shader0() noexcept {
    Pipeline *rp = pipeline();
    const Geometry &geometry = rp->geometry();
    LightSampler *light_sampler = scene().light_sampler();
    Camera *camera = scene().camera().get();
    Sampler *sampler = scene().sampler();

    auto RIS = [&] (Uint2 pixel) {
        OCReservoir ret;
        for (int i = 0; i < M; ++i) {

        }
        return ret;
    };

    Kernel kernel = [&](Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();
        OCReservoir rsv = RIS(pixel);
    };
    _shader0 = device().compile(kernel, "generate initial candidates, "
                                        "check visibility,temporal reuse");
}

void ReSTIRDI::compile_shader1() noexcept {
    Camera *camera = scene().camera().get();
    Sampler *sampler = scene().sampler();
    Kernel kernel = [&](Uint frame_index) {

    };
    _shader1 = device().compile(kernel, "spatial reuse and shading");
}

void ReSTIRDI::prepare() noexcept {
    const Pipeline *rp = pipeline();
    _prev_reservoirs = device().create_buffer<Reservoir>(rp->pixel_num());
    _reservoirs = device().create_buffer<Reservoir>(rp->pixel_num());
}

CommandList ReSTIRDI::estimate() const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    ret << _shader0(rp->frame_index()).dispatch(rp->resolution());
    ret << _shader1(rp->frame_index()).dispatch(rp->resolution());
    return ret;
}

}// namespace vision