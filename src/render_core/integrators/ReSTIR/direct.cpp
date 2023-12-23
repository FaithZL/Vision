//
// Created by Zero on 2023/9/15.
//

#include "direct.h"
#include "base/integrator.h"
#include "base/mgr/pipeline.h"

namespace vision {

ReSTIRDirectIllumination::ReSTIRDirectIllumination(IlluminationIntegrator *integrator, const ParameterSet &desc,
                                                   RegistrableManaged<float2> &motion_vec,
                                                   RegistrableManaged<SurfaceData> &surfaces0,
                                                   RegistrableManaged<SurfaceData> &surfaces1)
    : _integrator(integrator),
      M(desc["M"].as_uint(1)),
      _bsdf_num(desc["bsdf_num"].as_uint(1)),
      _spatial(desc["spatial"]),
      _temporal(desc["temporal"]),
      _correct_mode(static_cast<CorrectMode>(desc["correct_mode"].as_int(0))),
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
    return pipeline()->buffer<Reservoir>((_frame_index.value() % 2) + reservoir_base());
}

ResourceArrayBuffer<Reservoir> ReSTIRDirectIllumination::passthrough_reservoir() const noexcept {
    return pipeline()->buffer<Reservoir>(2 + reservoir_base());
}

ResourceArrayBuffer<Reservoir> ReSTIRDirectIllumination::cur_reservoir() const noexcept {
    return pipeline()->buffer<Reservoir>(((_frame_index.value() + 1) % 2) + reservoir_base());
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

SampledSpectrum ReSTIRDirectIllumination::sample_Li(const Interaction &it, MaterialEvaluator *bsdf,
                                                    const SampledWavelengths &swl, DIRSVSample *rsv_sample,
                                                    BSDFSample *output_bs) noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = *scene().spectrum();
    Sampler *sampler = scene().sampler();
    const Geometry &geometry = pipeline()->geometry();
    SampledSpectrum f{swl.dimension()};
    LightSample ls{swl.dimension()};
    BSDFSample bsdf_sample{swl.dimension()};
    auto sample_bsdf = [&](MaterialEvaluator *bsdf) {
        bsdf_sample = bsdf->sample(it.wo, sampler);
        RayState rs = it.spawn_ray_state(bsdf_sample.wi);
        OCHit hit = geometry.trace_closest(rs.ray);
        $if(hit->is_hit()) {
            Interaction next_it = geometry.compute_surface_interaction(hit, true);
            $if(next_it.has_emission()) {
                LightSampleContext p_ref;
                LightEval eval = light_sampler->evaluate_hit_point(it, next_it, swl);
                f = bsdf_sample.eval.f * eval.L;
                rsv_sample->light_index = light_sampler->combine_to_light_index(it.light_type_id(),
                                                                                it.light_inst_id());
                Float u_remapped = 0.f;
                light_sampler->dispatch_light(it.light_id(), [&](const Light *light) {
                    const IAreaLight *area_light = dynamic_cast<const IAreaLight *>(light);
                    if (!area_light) {
                        return;
                    }
                    u_remapped = area_light->combine(next_it.prim_id, it.uv.x);
                });
                rsv_sample->u = make_float2(u_remapped, it.uv.x);
                (*rsv_sample)->set_pos(next_it.pos);
            };
        };
    };
    if (!bsdf) {
        outline([&] {
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                MaterialEvaluator bsdf = material->create_evaluator(it, swl);
                swl.check_dispersive(spectrum, bsdf);
                sample_bsdf(&bsdf);
            });
        },
                "ReSTIRDirectIllumination::sample_Li from BSDF");
    } else {
        outline([&] {
            sample_bsdf(bsdf);
        },
                "ReSTIRDirectIllumination::sample_Li from BSDF");
    }
    if (output_bs) {
        *output_bs = bsdf_sample;
    }
    return f;
}

SampledSpectrum ReSTIRDirectIllumination::sample_Li(const Interaction &it, MaterialEvaluator *bsdf, const SampledWavelengths &swl,
                                                    const DIRSVSample &sample, LightSample *output_ls) noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = *scene().spectrum();
    SampledSpectrum f{swl.dimension()};
    LightSample ls{swl.dimension()};
    SampledLight sampled_light;
    sampled_light.light_index = sample.light_index;
    sampled_light.PMF = light_sampler->PMF(it, sample.light_index);
    ls = light_sampler->sample_point(sampled_light, it, sample.u, swl);
    Float3 wi = normalize(ls.p_light - it.pos);
    ScatterEval eval{swl.dimension()};
    if (!bsdf) {
        outline([&] {
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                MaterialEvaluator bsdf = material->create_evaluator(it, swl);
                swl.check_dispersive(spectrum, bsdf);
                eval = bsdf.evaluate(it.wo, wi);
            });
            f = eval.f * ls.eval.L;
        },
                "ReSTIRDirectIllumination::sample_Li from light");
    } else {
        outline([&] {
            eval = bsdf->evaluate(it.wo, wi);
            f = eval.f * ls.eval.L;
        },
                "ReSTIRDirectIllumination::sample_Li from light");
    }
    if (output_ls) {
        *output_ls = ls;
    }
    return f;
}

DIReservoir ReSTIRDirectIllumination::RIS(Bool hit, const Interaction &it, SampledWavelengths &swl,
                                          const Uint &frame_index) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = *scene().spectrum();
    comment("RIS start");
    DIReservoir ret;
    Float final_p_hat{0.f};

    auto sample_light = [&](MaterialEvaluator *bsdf) {
        SampledLight sampled_light = light_sampler->select_light(it, sampler->next_1d());
        DIRSVSample sample;
        sample.light_index = sampled_light.light_index;
        sample.u = sampler->next_2d();
        LightSample ls{swl.dimension()};
        Float p_hat = compute_p_hat(it, bsdf, swl, sample, std::addressof(ls));
        sample->set_pos(ls.p_light);
        Bool replace = ret->update(sampler->next_1d(), p_hat, ls.eval.pdf, sample);
//        $condition_info("light {} / {} = {}", p_hat, ls.eval.pdf, p_hat / ls.eval.pdf);
        final_p_hat = ocarina::select(replace, p_hat, final_p_hat);
    };

    auto sample_bsdf = [&](MaterialEvaluator *bsdf) {
        DIRSVSample sample;
        sample->init();
        BSDFSample bs{swl.dimension()};
        Float p_hat = compute_p_hat(it, bsdf, swl, addressof(sample), addressof(bs));
        Bool replace = ret->update(sampler->next_1d(), p_hat, bs.eval.pdf, sample);
        $condition_info("bsdf {} / {} = {}", p_hat, bs.eval.pdf, p_hat / bs.eval.pdf);
        final_p_hat = ocarina::select(replace, p_hat, final_p_hat);
    };

    $if(hit) {
        if (_integrator->separate()) {
            MaterialEvaluator bsdf(it, swl);
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                material->build_evaluator(bsdf, it, swl);
                swl.check_dispersive(spectrum, bsdf);
            });
            $for(i, M) {
                sample_light(addressof(bsdf));
            };
//            $for(i, _bsdf_num) {
//                sample_bsdf(addressof(bsdf));
//            };
        } else {
            $for(i, M) {
                sample_light(nullptr);
            };
            $condition_info("light ------weight {}", ret.weight_sum / ret.M);
//            $for(i, _bsdf_num) {
//                sample_bsdf(nullptr);
//            };
        }
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
        DIReservoir rsv = passthrough_reservoir().read(idx);
        Float p_hat = compute_p_hat(it, nullptr, swl, rsv.sample);
        cur_rsv->update(sampler->next_1d(), rsv, p_hat);
    });
    Float p_hat = compute_p_hat(it, nullptr, swl, cur_rsv.sample);
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
    Float p_hat = compute_p_hat(it, nullptr, swl, r1.sample, nullptr);
    ret->update(u, r1, p_hat);
    p_hat = compute_p_hat(it, nullptr, swl, ret.sample, nullptr);
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
    Uint limit = rsv.M * _temporal.limit;
    int2 res = make_int2(pipeline()->resolution());
    $if(in_screen(make_int2(prev_p_film), res)) {
        Uint index = dispatch_id(make_uint2(prev_p_film));
        DIReservoir prev_rsv = prev_reservoir().read(index);
        prev_rsv->truncation(limit);
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
        passthrough_reservoir().write(dispatch_id(), rsv);
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
    Container<uint> rsv_idx{_spatial.sample_num};
    $for(i, _spatial.sample_num) {
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

Float3 ReSTIRDirectIllumination::shading(vision::DIReservoir &rsv, const OCHit &hit,
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
            if (!light->match(LightType::Area)) { return; }
            LightSampleContext p_ref;
            p_ref.pos = camera->device_position();
            LightEval le = light->evaluate_wi(p_ref, it, swl);
            Le = le.L;
        });
    }
    $else {
        it.wo = normalize(camera->device_position() - it.pos);
        value = sample_Li(it, nullptr, swl, rsv.sample);
        Bool occluded = geometry.occluded(it, rsv.sample->p_light());
        rsv->process_occluded(occluded);
        value = value * rsv.W;
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
        DIReservoir temporal_rsv = passthrough_reservoir().read(dispatch_id());
        DIReservoir st_rsv = spatial_reuse(temporal_rsv, cur_surf, make_int2(pixel), swl, frame_index);
        Var hit = cur_surf.hit;
        Float3 L = make_float3(0.f);
        $if(!hit->is_miss()) {
            L = shading(st_rsv, hit, swl, frame_index);
        };
        film->add_sample(pixel, L, frame_index);
        cur_reservoir().write(dispatch_id(), st_rsv);
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