//
// Created by Zero on 2023/9/15.
//

#include "direct.h"
#include "base/mgr/pipeline.h"

namespace vision {

OCReservoir ReSTIRDI::RIS(Uint2 pixel, Bool hit, const Interaction &it, SampledWavelengths &swl) const noexcept {
    Pipeline *rp = pipeline();
    LightSampler *light_sampler = scene().light_sampler();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = rp->spectrum();
    OCReservoir ret;
    $if(hit) {
        for (int i = 0; i < M; ++i) {
            SampledLight sampled_light = light_sampler->select_light(it, sampler->next_1d());
            OCRSVSample sample;
            sample.light_index = sampled_light.light_index;
            sample.PMF = sampled_light.PMF;
            sample.u = sampler->next_2d();
            LightSample ls = light_sampler->sample(sampled_light, it, sample.u, swl);
            Float3 wi = normalize(ls.p_light - it.pos);
            SampledSpectrum f{swl.dimension()};
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                BSDF bsdf = material->compute_BSDF(it, swl);
                if (auto dispersive = spectrum.is_dispersive(&bsdf)) {
                    $if(*dispersive) {
                        swl.invalidation_secondary();
                    };
                }
                ScatterEval eval = bsdf.evaluate(it.wo, wi);
                f = eval.f;
            });
            f = f * ls.eval.L;
            Float pq = f.average();
            Float weight = pq / sample.PMF;
            ret->update(sampler->next_1d(), weight, sample);
        }
    };
    return ret;
}

void ReSTIRDI::compile_shader0() noexcept {
    Pipeline *rp = pipeline();
    const Geometry &geometry = rp->geometry();
    Camera *camera = scene().camera().get();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = rp->spectrum();

    Kernel kernel = [&](Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();

        sampler->start_pixel_sample(pixel, frame_index, 0);
        camera->load_data();
        SampledWavelengths swl = spectrum.sample_wavelength(sampler);
        SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);
        Var hit = geometry.trace_closest(rs.ray);
        Interaction it;
        $if(!hit->is_miss()) {
            it = geometry.compute_surface_interaction(hit, rs.ray, false);
        };
        OCReservoir rsv = RIS(pixel, !hit->is_miss(), it, swl);

        $if(!hit->is_miss()) {
            Bool occluded = geometry.occluded(it, rsv.sample->p_light());
            rsv.weight_sum = select(occluded, 0.f, rsv.weight_sum);
        };

        _reservoirs.write(dispatch_id(), rsv);
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