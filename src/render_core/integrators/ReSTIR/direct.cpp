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
    Spectrum &spectrum = rp->spectrum();

    auto RIS = [&](Uint2 pixel, OCHit hit, RayState rs, SampledWavelengths &swl) {
        OCReservoir ret;
        $if(!hit->is_miss()) {
            for (int i = 0; i < M; ++i) {
                Interaction it = geometry.compute_surface_interaction(hit, false);
                SampledLight sampled_light = light_sampler->select_light(it, sampler->next_1d());
                OCRSVSample sample;
                sample.light_index = sampled_light.light_index;
                sample.PMF = sampled_light.PMF;
                sample.u = sampler->next_2d();
                LightSample ls = light_sampler->sample(sampled_light, it, sample.u, swl);
                Float3 wi = normalize(ls.p_light - it.pos);
                Float pq = 1.f;
                scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                    BSDF bsdf = material->compute_BSDF(it, swl);
                    if (auto dispersive = spectrum.is_dispersive(&bsdf)) {
                        $if(*dispersive) {
                            swl.invalidation_secondary();
                        };
                    }

                });
            }
        };
        return ret;
    };

    Kernel kernel = [&](Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();
        sampler->start_pixel_sample(pixel, frame_index, 0);
        camera->load_data();
        SampledWavelengths swl = spectrum.sample_wavelength(sampler);
        SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);
        Var hit = geometry.trace_closest(rs.ray);
        OCReservoir rsv = RIS(pixel, hit, rs, swl);
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