//
// Created by Zero on 2023/10/29.
//

#include "indirect.h"
#include "base/integrator.h"

namespace vision {

ReSTIRIndirectIllumination::ReSTIRIndirectIllumination(RayTracingIntegrator *integrator,
                                                       const vision::ParameterSet &desc)
    : _integrator(integrator),
      _spatial(desc["spatial"]),
      _temporal(desc["temporal"]) {
}

void ReSTIRIndirectIllumination::init_sample(const Interaction &it, SampledWavelengths &swl) noexcept {
    Camera *camera = scene().camera().get();
    Film *film = camera->radiance_film();
    LightSampler *light_sampler = scene().light_sampler();

    OCRay ray = _integrator->rays().read(dispatch_id());

    Uint2 pixel = dispatch_idx().xy();
    sampler()->start(pixel, *_frame_index, 3);

}

void ReSTIRIndirectIllumination::compile_shader0() noexcept {
    Spectrum &spectrum = pipeline()->spectrum();
    Camera *camera = scene().camera().get();
    Kernel kernel = [&](Uint frame_index) {
        _frame_index.emplace(frame_index);
        OCSurfaceData surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            $return();
        };
        camera->load_data();
        Uint2 pixel = dispatch_idx().xy();
        sampler()->start(pixel, frame_index, 0);
        SampledWavelengths swl = spectrum.sample_wavelength(sampler());
        Interaction it = pipeline()->compute_surface_interaction(surf.hit, camera->device_position());
        init_sample(it, swl);
    };
    _shader0 = async([&, kernel = ocarina::move(kernel)] {
        return device().compile(kernel, "indirect initial samples and temporal reuse");
    });
}

void ReSTIRIndirectIllumination::compile_shader1() noexcept {
    Camera *camera = scene().camera().get();
    Film *film = camera->radiance_film();
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = pipeline()->spectrum();

    Kernel kernel = [&](Uint frame_index) {
        _frame_index.emplace(frame_index);
        OCSurfaceData surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            $return();
        };
    };
    _shader1 = async([&, kernel = ocarina::move(kernel)] {
        return device().compile(kernel, "indirect spatial reuse and shading");
    });
}

CommandList ReSTIRIndirectIllumination::estimate(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    ret << _shader0.get()(frame_index).dispatch(rp->resolution());
    ret << _shader1.get()(frame_index).dispatch(rp->resolution());
    return ret;
}

void ReSTIRIndirectIllumination::prepare() noexcept {
    using ReSTIRIndirect::Reservoir;
    Pipeline *rp = pipeline();

    _reservoirs.super() = device().create_buffer<Reservoir>(rp->pixel_num() * 2,
                                                            "ReSTIRIndirectIllumination::_reservoirs x 2");
    _reservoirs.register_self(0, rp->pixel_num());
    _reservoirs.register_view(rp->pixel_num(), rp->pixel_num());
    vector<Reservoir> host{rp->pixel_num() * 2, Reservoir{}};
    _reservoirs.upload_immediately(host.data());

    using ReSTIRIndirect::RSVSample;
    _init_samples.super() = device().create_buffer<RSVSample>(rp->pixel_num(),
                                                              "ReSTIRIndirectIllumination::_init_samples");
    _init_samples.register_self();
    vector<RSVSample> vec{rp->pixel_num(), RSVSample{}};
    _init_samples.upload_immediately(vec.data());
}

}// namespace vision