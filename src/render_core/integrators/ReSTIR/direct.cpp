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
      M_light(desc["M_light"].as_uint(10)),
      M_bsdf(desc["M_bsdf"].as_uint(1)),
      _spatial(desc["spatial"]),
      _temporal(desc["temporal"]),
      _correct_mode(static_cast<CorrectMode>(desc["correct_mode"].as_int(0))),
      _debias(desc["debias"].as_bool(false)),
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

SampledSpectrum ReSTIRDirectIllumination::Li(const Interaction &it, MaterialEvaluator *bsdf, const SampledWavelengths &swl,
                                             const DIRSVSample &sample, LightSample *output_ls) noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = *scene().spectrum();
    SampledSpectrum f{swl.dimension()};
    LightSample ls{swl.dimension()};
    $if(sample->valid()) {
        ls = light_sampler->evaluate_point(it, sample->lsp(), swl);
    };
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
    ret->init();

    auto sample_light = [&](MaterialEvaluator *bsdf) {
        DIRSVSample sample;
        sample->init();
        LightSurfacePoint lsp = light_sampler->sample_only(it, sampler);
        sample->set_lsp(lsp);
        LightSample ls{swl.dimension()};
        sample.p_hat = compute_p_hat(it, bsdf, swl, sample, std::addressof(ls));
        sample->set_pos(ls.p_light);
        Float weight = Reservoir::cal_weight(1.f / M_light, sample.p_hat, 1.f / ls.eval.pdf);
        ret->update(sampler->next_1d(), sample, weight);
    };

    $if(hit) {
        if (_integrator->separate()) {
            MaterialEvaluator bsdf(it, swl);
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                material->build_evaluator(bsdf, it, swl);
                swl.check_dispersive(spectrum, bsdf);
            });
            $for(i, M_light) {
                sample_light(addressof(bsdf));
            };
        } else {
            $for(i, M_light) {
                sample_light(nullptr);
            };
        }
    };
    ret->update_W(ret.sample.p_hat);
    ret->truncation(1);
    comment("RIS end");
    return ret;
}

DIReservoir ReSTIRDirectIllumination::combine_spatial(DIReservoir cur_rsv,
                                                      SampledWavelengths &swl,
                                                      const Container<uint> &rsv_idx) const noexcept {
    Sampler *sampler = scene().sampler();
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    const Geometry &geom = pipeline()->geometry();
    OCSurfaceData cur_surf = cur_surface().read(dispatch_id());
    Interaction it = geom.compute_surface_interaction(cur_surf.hit, true);
    it.wo = normalize(c_pos - it.pos);

    DIReservoir ret;
    ret->init();
    Uint sample_num = rsv_idx.count() + 1;
    Float cur_weight = Reservoir::cal_weight(1.f / sample_num,
                                             cur_rsv.sample.p_hat, cur_rsv.W);
    ret->update(0.5f, cur_rsv.sample, cur_weight, cur_rsv.M);

    rsv_idx.for_each([&](const Uint &idx) {
        DIReservoir rsv = passthrough_reservoir().read(idx);
        rsv.sample.p_hat = compute_p_hat(it, nullptr, swl, rsv.sample);
        Float neighbor_weight = Reservoir::cal_weight(1.f / sample_num,
                                                      rsv.sample.p_hat, rsv.W);
        ret->update(sampler->next_1d(), rsv.sample, neighbor_weight, rsv.M);
    });
    ret->update_W(ret.sample.p_hat);
    return ret;
}

DIReservoir ReSTIRDirectIllumination::combine_temporal(const DIReservoir &cur_rsv,
                                                       OCSurfaceData cur_surf,
                                                       const DIReservoir &other_rsv,
                                                       SampledWavelengths &swl) const noexcept {
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    const Geometry &geom = pipeline()->geometry();
    Sampler *sampler = scene().sampler();

    Interaction it = geom.compute_surface_interaction(cur_surf.hit, true);
    it.wo = normalize(c_pos - it.pos);

    DIReservoir ret;
    ret->init();
    Float cur_weight = Reservoir::cal_weight(MIS_weight(cur_rsv.M, other_rsv.M),
                                             cur_rsv.sample.p_hat, cur_rsv.W);
    ret->update(0.5f, cur_rsv.sample, cur_weight, cur_rsv.M);

    auto other_sample = other_rsv.sample;
    other_sample.p_hat = compute_p_hat(it, nullptr, swl, other_rsv.sample);
    Float other_weight = Reservoir::cal_weight(MIS_weight(other_rsv.M, cur_rsv.M),
                                               other_sample.p_hat, other_rsv.W);
    ret->update(sampler->next_1d(), other_sample, other_weight, other_rsv.M);
    ret->update_W(ret.sample.p_hat);
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
            rsv = combine_temporal(rsv, cur_surf, prev_rsv, swl);
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
        sampler->start(pixel, frame_index, 0);
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
        another_pixel = ocarina::clamp(another_pixel, make_int2(0u), res - 1);
        Uint index = dispatch_id(another_pixel);
        OCSurfaceData other_surf = cur_surface().read(index);
        $if(is_neighbor(cur_surf, other_surf)) {
            rsv_idx.push_back(index);
        };
    };
    $if(cur_surf.hit->is_hit()) {
        rsv = combine_spatial(rsv, swl, rsv_idx);
    };
    return rsv;
}

Float3 ReSTIRDirectIllumination::shading(vision::DIReservoir rsv, const OCHit &hit,
                                         SampledWavelengths &swl, const Uint &frame_index) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = pipeline()->spectrum();
    const Camera *camera = scene().camera().get();
    const Geometry &geometry = pipeline()->geometry();
    Float3 c_pos = camera->device_position();
    SampledSpectrum value = {swl.dimension(), 0.f};
    SampledSpectrum Le = {swl.dimension(), 0.f};
    Interaction it = geometry.compute_surface_interaction(hit, true);
    it.wo = normalize(c_pos - it.pos);

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
        Interaction next_it;
        OCRay ray;
        OCHit hit;
        BSDFSample bs{swl.dimension()};
        scene().materials().dispatch(it.material_id(), [&](const Material *material) {
            auto bsdf = material->create_evaluator(it, swl);
            bs = bsdf.sample(it.wo, sampler);
        });
        ray = it.spawn_ray(bs.wi);
        hit = geometry.trace_closest(ray);
        $if(hit->is_hit()) {
            next_it = geometry.compute_surface_interaction(hit, ray, true);
            $if(next_it.has_emission()) {
                LightSampleContext p_ref;
                p_ref.pos = ray->origin();
                p_ref.ng = it.ng;
                LightEval eval = light_sampler->evaluate_hit_wi(p_ref, next_it, swl);
                value = eval.L * bs.eval.f / bs.eval.pdf;
            };
        };
        it.wo = normalize(camera->device_position() - it.pos);
        value = Li(it, nullptr, swl, rsv.sample);
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
        sampler->start(pixel, frame_index, 0);
        SampledWavelengths swl = spectrum.sample_wavelength(sampler);
        camera->load_data();
        sampler->start(pixel, frame_index, 1);
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
    using ReSTIRDirect::Reservoir;
    Pipeline *rp = pipeline();
    Reservoir rsv;
    vector<Reservoir> vec{rp->pixel_num(), rsv};

    auto init_rsv = [&](RegistrableBuffer<Reservoir> &reservoirs) {
        reservoirs.set_resource_array(rp->resource_array());
        reservoirs.super() = device().create_buffer<Reservoir>(rp->pixel_num());
        reservoirs.register_self();
        reservoirs.upload_immediately(vec.data());
    };

    init_rsv(_reservoirs0);
    init_rsv(_reservoirs1);
    init_rsv(_reservoirs2);
}

CommandList ReSTIRDirectIllumination::estimate(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    ret << _shader0(frame_index).dispatch(rp->resolution());
    ret << _shader1(frame_index).dispatch(rp->resolution());
    return ret;
}

}// namespace vision