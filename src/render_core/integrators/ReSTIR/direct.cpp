//
// Created by Zero on 2023/9/15.
//

#include "direct.h"
#include "base/mgr/pipeline.h"

namespace vision {

OCReservoir ReSTIR::RIS(Bool hit, const Interaction &it, SampledWavelengths &swl) const noexcept {
    Pipeline *rp = pipeline();
    LightSampler *light_sampler = scene().light_sampler();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = rp->spectrum();
    comment("RIS start");
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
    comment("RIS end");
    return ret;
}

void ReSTIR::compile_shader0() noexcept {
    Pipeline *rp = pipeline();
    const Geometry &geometry = rp->geometry();
    Camera *camera = scene().camera().get();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = rp->spectrum();

    Kernel kernel = [&](Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();
        sampler->start_pixel_sample(pixel, frame_index, 0);
        SampledWavelengths swl = spectrum.sample_wavelength(sampler);
        camera->load_data();
        SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);
        Var hit = geometry.trace_closest(rs.ray);
        Interaction it;
        $if(!hit->is_miss()) {
            it = geometry.compute_surface_interaction(hit, rs.ray, false);
        };
        OCReservoir rsv = RIS(!hit->is_miss(), it, swl);

        $if(!hit->is_miss()) {
            Bool occluded = geometry.occluded(it, rsv.sample->p_light());
            rsv.weight_sum = select(occluded, 0.f, rsv.weight_sum);
        };

        OCReservoir prev_rsv = _prev_reservoirs.read(dispatch_id());
        comment("temporal reuse");
        rsv->merge(prev_rsv, sampler->next_1d());
        _reservoirs.write(dispatch_id(), rsv);
    };
    _shader0 = device().compile(kernel, "generate initial candidates, "
                                        "check visibility,temporal reuse");
}

OCReservoir ReSTIR::spatial_reuse(const Uint2 &pixel) const noexcept {
    Sampler *sampler = scene().sampler();
    OCReservoir ret;
    uint2 res = pipeline()->resolution();
    Uint min_x = max(0u, pixel.x - _spatial);
    Uint max_x = min(pixel.x + _spatial, res.x - 1);
    Uint min_y = max(0u, pixel.y - _spatial);
    Uint max_y = min(pixel.y + _spatial, res.y - 1);
    $for(x, min_x, max_x) {
        $for(y, min_y, max_y) {
            Uint index = y * res.x + x;
            OCReservoir rsv = _reservoirs.read(index);
            ret->merge(rsv, sampler->next_1d());
        };
    };
    return ret;
}

Float3 ReSTIR::shading(const vision::OCReservoir &rsv, SampledWavelengths &swl) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = pipeline()->spectrum();
    SampledSpectrum value = {swl.dimension(), 0.f};

    return spectrum.linear_srgb(value, swl);
}

void ReSTIR::compile_shader1() noexcept {
    Camera *camera = scene().camera().get();
    Film *film = camera->radiance_film();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = pipeline()->spectrum();
    Kernel kernel = [&](Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();
        sampler->start_pixel_sample(pixel, frame_index, 0);
        SampledWavelengths swl = spectrum.sample_wavelength(sampler);
        sampler->start_pixel_sample(pixel, frame_index, 1);
        OCReservoir rsv = spatial_reuse(pixel);
        Float3 L = shading(rsv, swl);
        film->update_sample(pixel, L, frame_index);
        _prev_reservoirs.write(dispatch_id(), rsv);
    };
    _shader1 = device().compile(kernel, "spatial reuse and shading");
}

void ReSTIR::prepare() noexcept {
    const Pipeline *rp = pipeline();
    _prev_reservoirs = device().create_buffer<Reservoir>(rp->pixel_num());
    _reservoirs = device().create_buffer<Reservoir>(rp->pixel_num());
    GBuffer = device().create_buffer<GData>(rp->pixel_num());
}

CommandList ReSTIR::estimate() const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    ret << _shader0(rp->frame_index()).dispatch(rp->resolution());
    ret << _shader1(rp->frame_index()).dispatch(rp->resolution());
    return ret;
}

}// namespace vision