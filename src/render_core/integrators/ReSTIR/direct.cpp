//
// Created by Zero on 2023/9/15.
//

#include "direct.h"
#include "base/mgr/pipeline.h"

namespace vision {

ReSTIRDirectIllumination::ReSTIRDirectIllumination(const ParameterSet &desc, RegistrableManaged<ocarina::float2> &motion_vec,
                                                   RegistrableManaged<SurfaceData> &surfaces,
                                                   RegistrableManaged<SurfaceData> &prev_surfaces)
    : M(desc["M"].as_uint(1)),
      _spatial(desc["spatial"]),
      _temporal(desc["temporal"], pipeline()->resolution()),
      _mis(desc["mis"].as_bool(false)),
      _motion_vectors(motion_vec),
      _surfaces(surfaces),
      _prev_surfaces(prev_surfaces) {}

Bool ReSTIRDirectIllumination::is_neighbor(const OCSurfaceData &cur_surface,
                                           const OCSurfaceData &another_surface) const noexcept {
    return vision::is_neighbor(cur_surface, another_surface,
                               _spatial.dot_threshold,
                               _spatial.depth_threshold);
}

Bool ReSTIRDirectIllumination::is_temporal_valid(const OCSurfaceData &cur_surface,
                                                 const OCSurfaceData &prev_surface) const noexcept {
    return vision::is_neighbor(cur_surface, prev_surface,
                               _temporal.dot_threshold,
                               _temporal.depth_threshold);
}

Float ReSTIRDirectIllumination::compute_p_hat(const vision::Interaction &it,
                                              vision::SampledWavelengths &swl,
                                              const vision::DIRSVSample &sample,
                                              vision::LightSample *output_ls) noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = *scene().spectrum();
    SampledLight sampled_light;
    sampled_light.light_index = sample.light_index;
    sampled_light.PMF = light_sampler->PMF(it, sample.light_index);
    LightSample ls = light_sampler->sample(sampled_light, it, sample.u, swl);
    Float3 wi = normalize(ls.p_light - it.pos);
    SampledSpectrum f{swl.dimension()};
    ScatterEval eval{swl.dimension()};
    scene().materials().dispatch(it.material_id(), [&](const Material *material) {
        BSDF bsdf = material->compute_BSDF(it, swl);
        if (auto dispersive = spectrum.is_dispersive(&bsdf)) {
            $if(*dispersive) {
                swl.invalidation_secondary();
            };
        }
        eval = bsdf.evaluate(it.wo, wi);
    });
    f = eval.f * ls.eval.L;
    if (output_ls) {
        *output_ls = ls;
    }
    Float p_hat = f.average();
    return p_hat;
}

DIReservoir ReSTIRDirectIllumination::RIS(Bool hit, const Interaction &it, SampledWavelengths &swl,
                                          const Uint &frame_index) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Sampler *sampler = scene().sampler();
    comment("RIS start");
    DIReservoir ret;
    $if(hit) {
        for (int i = 0; i < M; ++i) {
            SampledLight sampled_light = light_sampler->select_light(it, sampler->next_1d());
            DIRSVSample sample;
            sample.light_index = sampled_light.light_index;
            sample.u = sampler->next_2d();
            LightSample ls{swl.dimension()};
            Float p_hat = compute_p_hat(it, swl, sample, &ls);
            sample.p_hat = p_hat;
            sample.pdf = ls.eval.pdf;
            sample->set_pos(ls.p_light);
            ret->update(sampler->next_1d(), sample);
        }
    };
    ret->update_W();
    comment("RIS end");
    return ret;
}

DIReservoir ReSTIRDirectIllumination::combine_reservoirs_MIS(DIReservoir cur_rsv,
                                                             SampledWavelengths &swl,
                                                             const Container<uint> &rsv_idx) const noexcept {
    Sampler *sampler = scene().sampler();
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    const Geometry &geom = pipeline()->geometry();
    Float p_sum = 0.f;
    OCSurfaceData cur_surf = _surfaces.read(dispatch_id());
    Interaction it = geom.compute_surface_interaction(cur_surf.hit, true);
    it.wo = normalize(c_pos - it.pos);
    Float temp_M = cur_rsv.M;

    rsv_idx.for_each([&](const Uint &idx) {
        DIReservoir rsv = _reservoirs.read(idx);
        Float p_hat = compute_p_hat(it, swl, rsv.sample);
        cur_rsv->update(sampler->next_1d(), rsv);
        p_sum += p_hat * rsv.M;
    });

    cur_rsv.sample.p_hat = compute_p_hat(it, swl, cur_rsv.sample);
    p_sum += cur_rsv.sample.p_hat * temp_M;
    cur_rsv->update_W_MIS(p_sum);
    return cur_rsv;
}

DIReservoir ReSTIRDirectIllumination::combine_reservoir_MIS(DIReservoir r0,
                                                            OCSurfaceData s0,
                                                            DIReservoir r1,
                                                            OCSurfaceData s1,
                                                            SampledWavelengths &swl) const noexcept {
    Sampler *sampler = scene().sampler();
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    const Geometry &geom = pipeline()->geometry();

    Interaction it = geom.compute_surface_interaction(s0.hit, true);
    it.wo = normalize(c_pos - it.pos);

    Float p_hat_00 = compute_p_hat(it, swl, r0.sample);
    Float p_hat_01 = compute_p_hat(it, swl, r1.sample);

    DIReservoir rsv = r0;
    Float p_sum = p_hat_00 * r0.M + p_hat_01 * r1.M;
    Bool replace = rsv->update(sampler->next_1d(), r1);

    $if(!replace) {
        rsv.sample.p_hat = p_hat_00;
    }
    $else {
        rsv.sample.p_hat = p_hat_01;
    };
    rsv->update_W_MIS(p_sum);
    return rsv;
}

DIReservoir ReSTIRDirectIllumination::combine_reservoirs(DIReservoir cur_rsv,
                                                         SampledWavelengths &swl,
                                                         const Container<uint> &rsv_idx) const noexcept {
    Sampler *sampler = scene().sampler();
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    const Geometry &geom = pipeline()->geometry();
    OCSurfaceData cur_surf = _surfaces.read(dispatch_id());
    Interaction it = geom.compute_surface_interaction(cur_surf.hit, true);
    it.wo = normalize(c_pos - it.pos);
    rsv_idx.for_each([&](const Uint &idx) {
        DIReservoir rsv = _reservoirs.read(idx);
        cur_rsv->update(sampler->next_1d(), rsv);
    });
    cur_rsv.sample.p_hat = compute_p_hat(it, swl, cur_rsv.sample);
    cur_rsv->update_W();
    return cur_rsv;
}

DIReservoir ReSTIRDirectIllumination::combine_reservoir(const DIReservoir &r0,
                                                        const DIReservoir &r1,
                                                        SampledWavelengths &swl) const noexcept {
    DIReservoir ret;
    ret = r0;
    Float u = scene().sampler()->next_1d();
    ret->update(u, r1);
    ret->update_W();
    return ret;
}

Float2 ReSTIRDirectIllumination::compute_motion_vec(const Float2 &p_film, const Float3 &cur_pos, const Bool &is_hit) const noexcept {
    Camera *camera = scene().camera().get();
    Float2 ret = make_float2(0.f);
    $if(is_hit) {
        Float2 raster_coord = camera->prev_raster_coord(cur_pos).xy();
        ret = p_film - raster_coord;
    };
    return ret;
}

void ReSTIRDirectIllumination::compile_shader0() noexcept {
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

        OCSurfaceData cur_surf;
        cur_surf.hit = hit;
        cur_surf->set_t_max(0.f);
        Interaction it;
        $if(hit->is_hit()) {
            it = geometry.compute_surface_interaction(hit, rs.ray, true);
            cur_surf->set_t_max(rs.t_max());
            cur_surf->set_normal(it.ng);
        };
        DIReservoir rsv = RIS(hit->is_hit(), it, swl, frame_index);
        Float2 motion_vec = compute_motion_vec(ss.p_film, it.pos, hit->is_hit());

        _motion_vectors.write(dispatch_id(), motion_vec);
        $if(hit->is_hit()) {
            Bool occluded = geometry.occluded(it, rsv.sample->p_light());
            rsv->process_occluded(occluded);
        };
        _reservoirs.write(dispatch_id(), rsv);
        _surfaces.write(dispatch_id(), cur_surf);
    };
    _shader0 = device().compile(kernel, "generate initial candidates and "
                                        "check visibility");
}

DIReservoir ReSTIRDirectIllumination::spatial_reuse(DIReservoir rsv, const OCSurfaceData &cur_surf,
                                                    const Int2 &pixel, SampledWavelengths &swl,
                                                    const Uint &frame_index) const noexcept {
    if (!_spatial.open) {
        return rsv;
    }
    Sampler *sampler = scene().sampler();
    int2 res = make_int2(pipeline()->resolution());
    Container<uint> rsv_idx{_spatial.iterate_num};
    $for(i, _spatial.iterate_num) {
        Float2 offset = square_to_disk(sampler->next_2d()) * _spatial.sampling_radius;
        Int2 offset_i = make_int2(ocarina::round(offset));
        Int2 another_pixel = pixel + offset_i;
        $if(!in_screen(another_pixel, res)) {
            $continue;
        };
        Uint index = dispatch_id(another_pixel);
        OCSurfaceData other_surf = _surfaces.read(index);
        $if(is_neighbor(cur_surf, other_surf)) {
            rsv_idx.push_back(index);
        };
    };
    $if(cur_surf.hit->is_hit()) {
        if (_mis) {
            rsv = combine_reservoirs_MIS(rsv, swl, rsv_idx);
        } else {
            rsv = combine_reservoirs(rsv, swl, rsv_idx);
        }
    };
    return rsv;
}

DIReservoir ReSTIRDirectIllumination::temporal_reuse(DIReservoir rsv, const OCSurfaceData &cur_surf,
                                                     const SensorSample &ss,
                                                     SampledWavelengths &swl,
                                                     const Uint &frame_index) const noexcept {
    if (!_temporal.open) {
        return rsv;
    }
    Float2 motion_vec = _motion_vectors.read(dispatch_id());
    Float2 prev_p_film = ss.p_film - motion_vec;
    int2 res = make_int2(pipeline()->resolution());
    $if(in_screen(make_int2(prev_p_film), res)) {
        Uint index = dispatch_id(make_uint2(prev_p_film));
        DIReservoir prev_rsv = _prev_reservoirs.read(index);
        prev_rsv->truncation(_temporal.limit);
        OCSurfaceData another_surf = _prev_surfaces.read(index);
        $if(is_temporal_valid(cur_surf, another_surf)) {
            rsv = combine_reservoir(rsv, prev_rsv, swl);
        };
    };
    return rsv;
}

Float3 ReSTIRDirectIllumination::shading(const vision::DIReservoir &rsv, const OCHit &hit,
                                         SampledWavelengths &swl, const Uint &frame_index) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = pipeline()->spectrum();
    const Camera *camera = scene().camera().get();
    const Geometry &geometry = pipeline()->geometry();

    SampledSpectrum value = {swl.dimension(), 0.f};
    SampledSpectrum Le = {swl.dimension(), 0.f};
    Interaction it = geometry.compute_surface_interaction(hit, true);

    $if(it.has_emission()) {
        light_sampler->dispatch_light(it.light_id(), [&](const Light *light) {
            if (light->type() != LightType::Area) { return; }
            LightSampleContext p_ref;
            p_ref.pos = camera->device_position();
            LightEval le = light->evaluate(p_ref, it, swl);
            Le = le.L;
        });
    }
    $else {
        SampledLight sampled_light;
        sampled_light.light_index = rsv.sample.light_index;
        LightSample ls = light_sampler->sample(sampled_light, it, rsv.sample.u, swl);
        Float3 wo = normalize(camera->device_position() - it.pos);
        Float3 wi = normalize(rsv.sample->p_light() - it.pos);
        scene().materials().dispatch(it.material_id(), [&](const Material *material) {
            BSDF bsdf = material->compute_BSDF(it, swl);
            if (auto dispersive = spectrum.is_dispersive(&bsdf)) {
                $if(*dispersive) {
                    swl.invalidation_secondary();
                };
            }
            ScatterEval se = bsdf.evaluate(wo, wi);
            value = ls.eval.L * se.f;
            value = value * rsv.W;
        });
    };
    return spectrum.linear_srgb(value + Le, swl);
}

void ReSTIRDirectIllumination::compile_shader1() noexcept {
    Camera *camera = scene().camera().get();
    Film *film = camera->radiance_film();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = pipeline()->spectrum();
    Kernel kernel = [&](Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();
        camera->load_data();
        sampler->start_pixel_sample(pixel, frame_index, 0);
        SampledWavelengths swl = spectrum.sample_wavelength(sampler);
        camera->load_data();
        SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
        sampler->start_pixel_sample(pixel, frame_index, 1);
        DIReservoir cur_rsv = _reservoirs.read(dispatch_id());
        OCSurfaceData cur_surf = _surfaces.read(dispatch_id());
        DIReservoir temporal_rsv = temporal_reuse(cur_rsv, cur_surf, ss, swl, frame_index);
        _reservoirs.write(dispatch_id(), temporal_rsv);
        DIReservoir st_rsv = spatial_reuse(temporal_rsv, cur_surf, make_int2(pixel), swl, frame_index);
        Var hit = cur_surf.hit;
        Float3 L = make_float3(0.f);
        $if(!hit->is_miss()) {
            L = shading(st_rsv, hit, swl, frame_index);
        };
        Float l = L.x;
//                $if(l > 0.5f && dispatch_idx().y > 300) {
//                    Printer::instance().info_with_location("{} {} {} ========{} {} {} {}========", L, st_rsv.W, st_rsv.M, st_rsv.sample.p_hat, frame_index);
//                };
//                $if(all(dispatch_idx().xy() == make_uint2(519, 480))) {
//                    Printer::instance().info_with_location("{} {} {} ========{} {} {}========", L, st_rsv.W, st_rsv.M, st_rsv.sample.p_hat);
//                };
        film->update_sample(pixel, L, frame_index);
    };
    _shader1 = device().compile(kernel, "spatial temporal reuse and shading");
}

void ReSTIRDirectIllumination::prepare() noexcept {
    Pipeline *rp = pipeline();
    _prev_reservoirs.set_resource_array(rp->resource_array());
    _reservoirs.set_resource_array(rp->resource_array());

    _prev_reservoirs.reset_all(device(), rp->pixel_num());
    _reservoirs.reset_all(device(), rp->pixel_num());

    _prev_reservoirs.register_self();
    _reservoirs.register_self();
}

CommandList ReSTIRDirectIllumination::estimate(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    ret << _shader0(frame_index).dispatch(rp->resolution());
    ret << _shader1(frame_index).dispatch(rp->resolution());
    ret << _prev_reservoirs.copy_from(_reservoirs.device_buffer(), 0);
    ret << _prev_surfaces.copy_from(_surfaces.device_buffer(), 0);
    ret << _motion_vectors.download();
    return ret;
}

}// namespace vision