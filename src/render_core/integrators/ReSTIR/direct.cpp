//
// Created by Zero on 2023/9/15.
//

#include "direct.h"
#include "base/mgr/pipeline.h"

namespace vision {

ReSTIRDirectIllumination::ReSTIRDirectIllumination(const ParameterSet &desc, RegistrableManaged<ocarina::float2> &motion_vec,
                                                   RegistrableManaged<SurfaceData> &surfaces0,
                                                   RegistrableManaged<SurfaceData> &surfaces1)
    : M(desc["M"].as_uint(1)),
      _spatial(desc["spatial"]),
      _temporal(desc["temporal"], M, _spatial.iterate_num, pipeline()->resolution()),
      _mis(desc["mis"].as_bool(false)),
      _motion_vectors(motion_vec),
      _surfaces0(surfaces0),
      _surfaces1(surfaces1) {}

ResourceArrayBuffer<SurfaceData> ReSTIRDirectIllumination::prev_surface() const noexcept {
    return pipeline()->buffer<SurfaceData>((_frame_index.value() % 2) + surface_base());
}

ResourceArrayBuffer<SurfaceData> ReSTIRDirectIllumination::cur_surface() const noexcept {
    return pipeline()->buffer<SurfaceData>(((_frame_index.value() + 1) % 2) + surface_base());
}

ResourceArrayBuffer<Reservoir> ReSTIRDirectIllumination::prev_reservoir() const noexcept {
    return pipeline()->buffer<Reservoir>((_frame_index.value() % 3) + reservoir_base());
}

ResourceArrayBuffer<Reservoir> ReSTIRDirectIllumination::cur_reservoir() const noexcept {
    return pipeline()->buffer<Reservoir>(((_frame_index.value() + 2) % 3) + reservoir_base());
}

ResourceArrayBuffer<Reservoir> ReSTIRDirectIllumination::next_reservoir() const noexcept {
    return pipeline()->buffer<Reservoir>(((_frame_index.value() + 1) % 3) + reservoir_base());
}

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
    Float final_p_hat{0.f};
    $if(hit) {
        $for(i, M) {
            SampledLight sampled_light = light_sampler->select_light(it, sampler->next_1d());
            DIRSVSample sample;
            sample.light_index = sampled_light.light_index;
            sample.u = sampler->next_2d();
            LightSample ls{swl.dimension()};
            Float p_hat = compute_p_hat(it, swl, sample, &ls);
            sample->set_pos(ls.p_light);
            Bool replace = ret->update(sampler->next_1d(), p_hat, ls.eval.pdf, sample);
            final_p_hat = ocarina::select(replace, p_hat, final_p_hat);
        };
    };
    ret->update_W(final_p_hat);
    comment("RIS end");
    return ret;
}

DIReservoir ReSTIRDirectIllumination::combine_reservoirs(DIReservoir cur_rsv,
                                                         SampledWavelengths &swl,
                                                         const Container<uint> &rsv_idx) const noexcept {
    Sampler *sampler = scene().sampler();
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    const Geometry &geom = pipeline()->geometry();
    OCSurfaceData cur_surf = cur_surface().read(dispatch_id());
    Interaction it = geom.compute_surface_interaction(cur_surf.hit, true);
    it.wo = normalize(c_pos - it.pos);
    rsv_idx.for_each([&](const Uint &idx) {
        DIReservoir rsv = cur_reservoir().read(idx);
        Float p_hat = compute_p_hat(it, swl, rsv.sample);
        cur_rsv->update(sampler->next_1d(), rsv, p_hat);
    });
    Float p_hat = compute_p_hat(it, swl, cur_rsv.sample);
    cur_rsv->update_W(p_hat);
    return cur_rsv;
}

DIReservoir ReSTIRDirectIllumination::combine_reservoir(const DIReservoir &r0,
                                                        OCSurfaceData cur_surf,
                                                        const DIReservoir &r1,
                                                        SampledWavelengths &swl) const noexcept {
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    const Geometry &geom = pipeline()->geometry();
    DIReservoir ret;
    ret = r0;
    Float u = scene().sampler()->next_1d();
    Interaction it = geom.compute_surface_interaction(cur_surf.hit, true);
    it.wo = normalize(c_pos - it.pos);
    Float p_hat = compute_p_hat(it, swl, r1.sample);
    ret->update(u, r1, p_hat);
    p_hat = compute_p_hat(it, swl, ret.sample);
    ret->update_W(p_hat);
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

DIReservoir ReSTIRDirectIllumination::temporal_reuse(DIReservoir rsv, const OCSurfaceData &cur_surf,
                                                     const Float2 &motion_vec,
                                                     const SensorSample &ss,
                                                     SampledWavelengths &swl,
                                                     const Uint &frame_index) const noexcept {
    if (!_temporal.open) {
        return rsv;
    }
    Sampler *sampler = scene().sampler();
    Float2 prev_p_film = ss.p_film - motion_vec;
    prev_p_film += square_to_disk(sampler->next_2d()) * _temporal.sampling_radius;
    int2 res = make_int2(pipeline()->resolution());
    $if(in_screen(make_int2(prev_p_film), res)) {
        Uint index = dispatch_id(make_uint2(prev_p_film));
        DIReservoir prev_rsv = prev_reservoir().read(index);
        prev_rsv->truncation(_temporal.limit);
        OCSurfaceData another_surf = prev_surface().read(index);
        $if(is_temporal_valid(cur_surf, another_surf)) {
            rsv = combine_reservoir(rsv, cur_surf, prev_rsv, swl);
        };
    };
    return rsv;
}

void ReSTIRDirectIllumination::compile_shader0() noexcept {
    Pipeline *rp = pipeline();
    const Geometry &geometry = rp->geometry();
    Camera *camera = scene().camera().get();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = rp->spectrum();

    Kernel kernel = [&](Uint frame_index) {
        _frame_index.emplace(frame_index);
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
            cur_surf.mat_id = it.material_id();
            cur_surf->set_t_max(rs.t_max());
            cur_surf->set_normal(it.shading.normal());
        };
        DIReservoir rsv = RIS(hit->is_hit(), it, swl, frame_index);
        Float2 motion_vec = compute_motion_vec(ss.p_film, it.pos, hit->is_hit());

        _motion_vectors.write(dispatch_id(), motion_vec);

        $if(hit->is_hit()) {
            Bool occluded = geometry.occluded(it, rsv.sample->p_light());
            rsv->process_occluded(occluded);
        };
        rsv = temporal_reuse(rsv, cur_surf, motion_vec, ss, swl, frame_index);
        cur_reservoir().write(dispatch_id(), rsv);
        cur_surface().write(dispatch_id(), cur_surf);
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
        OCSurfaceData other_surf = cur_surface().read(index);
        $if(is_neighbor(cur_surf, other_surf)) {
            rsv_idx.push_back(index);
        };
    };
    $if(cur_surf.hit->is_hit()) {
        rsv = combine_reservoirs(rsv, swl, rsv_idx);
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
        _frame_index.emplace(frame_index);
        Uint2 pixel = dispatch_idx().xy();
        camera->load_data();
        sampler->start_pixel_sample(pixel, frame_index, 0);
        SampledWavelengths swl = spectrum.sample_wavelength(sampler);
        camera->load_data();
        sampler->start_pixel_sample(pixel, frame_index, 1);
        OCSurfaceData cur_surf = cur_surface().read(dispatch_id());
        DIReservoir temporal_rsv = cur_reservoir().read(dispatch_id());
        DIReservoir st_rsv = spatial_reuse(temporal_rsv, cur_surf, make_int2(pixel), swl, frame_index);
        Var hit = cur_surf.hit;
        Float3 L = make_float3(0.f);
        $if(!hit->is_miss()) {
            L = shading(st_rsv, hit, swl, frame_index);
        };
        film->add_sample(pixel, L, frame_index);
        next_reservoir().write(dispatch_id(), st_rsv);
    };
    _shader1 = device().compile(kernel, "spatial temporal reuse and shading");
}

void ReSTIRDirectIllumination::prepare() noexcept {
    Pipeline *rp = pipeline();
    _reservoirs0.set_resource_array(rp->resource_array());
    _reservoirs0.reset_all(device(), rp->pixel_num());
    _reservoirs0.register_self();

    _reservoirs1.set_resource_array(rp->resource_array());
    _reservoirs1.reset_all(device(), rp->pixel_num());
    _reservoirs1.register_self();

    _reservoirs2.set_resource_array(rp->resource_array());
    _reservoirs2.reset_all(device(), rp->pixel_num());
    _reservoirs2.register_self();
}

CommandList ReSTIRDirectIllumination::estimate(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    ret << _shader0(frame_index).dispatch(rp->resolution());
    ret << _shader1(frame_index).dispatch(rp->resolution());
    return ret;
}

}// namespace vision