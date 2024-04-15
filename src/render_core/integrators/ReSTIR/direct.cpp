//
// Created by Zero on 2023/9/15.
//

#include "direct.h"
#include "base/integrator.h"

namespace vision {

ReSTIRDI::ReSTIRDI(IlluminationIntegrator *integrator, const ParameterSet &desc)
    : _integrator(integrator),
      M_light(desc["M_light"].as_uint(10)),
      M_bsdf(desc["M_bsdf"].as_uint(1)),
      _spatial(desc["spatial"]),
      _temporal(desc["temporal"]),
      _debias(desc["debias"].as_bool(false)),
      _reweight(desc["reweight"].as_bool(false)),
      _pairwise(desc["pairwise"].as_bool(false)),
      _open(desc["open"].as_bool(true)) {}

bool ReSTIRDI::render_UI(ocarina::Widgets *widgets) noexcept {
    bool open = widgets->use_tree("ReSTIR DI", [&] {
        _changed |= widgets->input_uint_limit("M light", &M_light, 0, 100);
        _changed |= widgets->input_uint_limit("M BSDF", &M_bsdf, 0, 100);
        _changed |= widgets->input_uint_limit("history", &_temporal.limit, 0, 50, 1, 3);
        _changed |= widgets->check_box("temporal", &_temporal.open);
        if (_temporal.open) {
            _changed |= widgets->input_float_limit("temporal theta",
                                                   &_temporal.theta, 0, 90, 1, 1);
            _changed |= widgets->input_float_limit("temporal depth", &_temporal.depth_threshold,
                                                   0, 1, 0.02, 0.1);
            _changed |= widgets->input_float_limit("temporal radius", &_temporal.sampling_radius,
                                                   0, 50, 1, 5);
        }
        _changed |= widgets->check_box("spatial", &_spatial.open);
        if (_spatial.open) {
            _changed |= widgets->input_float_limit("spatial theta",
                                                   &_spatial.theta, 0, 90, 1, 1);
            _changed |= widgets->input_float_limit("spatial depth", &_spatial.depth_threshold,
                                                   0, 1, 0.02, 0.1);
            _changed |= widgets->input_float_limit("spatial radius", &_spatial.sampling_radius,
                                                   0, 50, 1, 5);
        }
    });
    return open;
}

SampledSpectrum ReSTIRDI::Li(const Interaction &it, MaterialEvaluator *bsdf, DIRSVSample *sample,
                             BSDFSample *bs, Float *light_pdf_point, HitBSDFVar *hit_bsdf) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = *scene().spectrum();
    const SampledWavelengths &swl = sampled_wavelengths();
    const Geometry &geometry = pipeline()->geometry();
    SampledSpectrum f{swl.dimension()};
    Sampler *sampler = scene().sampler();

    if (!bsdf) {
        outline([&] {
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                MaterialEvaluator bsdf = material->create_evaluator(it, swl);
                swl.check_dispersive(spectrum, bsdf);
                *bs = bsdf.sample(it.wo, sampler);
            });
        },
                "ReSTIRDI::Li from bsdf");
    } else {
        outline([&] {
            *bs = bsdf->sample(it.wo, sampler);
        },
                "ReSTIRDI::Li from bsdf");
    }
    RayVar ray = it.spawn_ray(bs->wi);
    HitVar hit = geometry.trace_closest(ray);
    Float pdf = bs->eval.pdf;
    hit_bsdf->next_ray = ray;
    hit_bsdf->next_hit = hit;
    hit_bsdf->bsdf.set(bs->eval.f.vec3());
    hit_bsdf->cos_theta = abs_dot(it.ng, bs->wi);
    hit_bsdf->pdf = pdf;

    LightEval le{swl.dimension()};
    LightSurfacePoint lsp;
    $if(hit->is_hit()) {
        Interaction next_it = geometry.compute_surface_interaction(hit, ray);
        $if(next_it.has_emission()) {
            /**
             * The PDF of the LightEval is the PDF of the reflection function
             * from the solid Angle space to the area space
             */
            le = light_sampler->evaluate_hit_point(it, next_it, bs->eval.pdf,
                                                   swl, light_pdf_point);
            lsp.light_index = light_sampler->extract_light_index(next_it);
            lsp.prim_id = hit.prim_id;
            lsp.bary = hit.bary;
            (*sample)->set_lsp(lsp);
            (*sample)->set_pos(next_it.robust_position(it.pos - next_it.pos));
            bs->eval.pdf = le.pdf;
            f = bs->eval.f * le.L;
        };
    }
    $else {
        if (light_sampler->env_light()) {
            LightSampleContext p_ref{it};
            Float3 wi = bs->wi * scene().world_diameter();
            le = light_sampler->evaluate_miss_point(p_ref, wi, bs->eval.pdf,
                                                    swl, light_pdf_point);
            lsp.light_index = light_sampler->env_index();
            lsp.bary = light_sampler->env_light()->convert_to_bary(bs->wi);
            (*sample)->set_lsp(lsp);
            (*sample)->set_pos(it.pos + wi);
            f = bs->eval.f * le.L;
        }
    };

    return f;
}

SampledSpectrum ReSTIRDI::Li(const Interaction &it, MaterialEvaluator *bsdf, const DIRSVSample &sample,
                             LightSample *output_ls, Float *bsdf_pdf_point) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = *scene().spectrum();
    const SampledWavelengths &swl = sampled_wavelengths();
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
                "ReSTIRDI::Li from light");
    } else {
        outline([&] {
            eval = bsdf->evaluate(it.wo, wi);
            f = eval.f * ls.eval.L;
        },
                "ReSTIRDI::Li from light");
    }
    if (bsdf_pdf_point) {
        *bsdf_pdf_point = light_sampler->PDF_point(it, sample->lsp(), eval.pdf);
    }
    if (output_ls) {
        *output_ls = ls;
    }
    return f;
}

DIReservoir ReSTIRDI::RIS(Bool hit, const Interaction &it,
                          const Var<Param> &param) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Sampler *sampler = scene().sampler();
    Spectrum &spectrum = *scene().spectrum();
    comment("RIS start");
    Uint M_light = param.M_light;
    Uint M_bsdf = param.M_bsdf;
    DIReservoir ret;
    const SampledWavelengths &swl = sampled_wavelengths();
    auto sample_light = [&](MaterialEvaluator *bsdf) {
        DIRSVSample sample;
        //todo merge sample and evaluate
        LightSurfacePoint lsp = light_sampler->sample_only(it, sampler);
        sample->set_lsp(lsp);
        LightSample ls{swl.dimension()};
        Float bsdf_light_point = 0.f;
        sample.p_hat = compute_p_hat(it, bsdf, sample, std::addressof(ls), &bsdf_light_point);
        sample->set_pos(ls.p_light);
        Bool is_delta_light = ls.eval.pdf < 0.f;
        Float light_pdf = ocarina::select(is_delta_light, -ls.eval.pdf, ls.eval.pdf);
        Float mis_weight = ocarina::select(is_delta_light, 1.f / M_light,
                                           light_pdf / (M_light * light_pdf + M_bsdf * bsdf_light_point));
        Float weight = Reservoir::safe_weight(mis_weight,
                                              sample.p_hat, 1.f / light_pdf);
        ret->update(sampler->next_1d(), sample, weight);
    };

    HitBSDFVar hit_bsdf;

    auto sample_bsdf = [&](MaterialEvaluator *bsdf) {
        DIRSVSample sample;
        BSDFSample bs{swl.dimension()};
        Float light_pdf_point = 0.f;
        Float p_hat = compute_p_hat(it, bsdf, &sample, &bs, &light_pdf_point, &hit_bsdf);
        sample.p_hat = p_hat;
        Float weight = Reservoir::safe_weight(bs.eval.pdf / (M_light * light_pdf_point + M_bsdf * bs.eval.pdf),
                                              sample.p_hat, 1.f / bs.eval.pdf);
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
            $for(i, M_bsdf) {
                sample_bsdf(addressof(bsdf));
            };

        } else {
            $for(i, M_light) {
                sample_light(nullptr);
            };
            $for(i, M_bsdf) {
                sample_bsdf(nullptr);
            };
        }
    };
    frame_buffer().hit_bsdfs().write(dispatch_id(), hit_bsdf);
    ret->update_W(ret.sample.p_hat);
    ret->truncation(1);
    comment("RIS end");
    return ret;
}

Float ReSTIRDI::neighbor_pairwise_MIS(const DIReservoir &canonical_rsv, const Interaction &canonical_it,
                                      const DIReservoir &other_rsv, const Interaction &other_it,
                                      Uint M, DIReservoir *output_rsv) const noexcept {
    Sampler *sampler = scene().sampler();
    const SampledWavelengths &swl = sampled_wavelengths();
    Float p_hat_c_at_c = compute_p_hat(canonical_it, nullptr, canonical_rsv.sample);
    Float p_hat_c_at_n = compute_p_hat(other_it, nullptr, canonical_rsv.sample);
    Float p_hat_n_at_n = compute_p_hat(other_it, nullptr, other_rsv.sample);
    Float p_hat_n_at_c = compute_p_hat(canonical_it, nullptr, other_rsv.sample);

    Float num = M - 1;

    Float mi = MIS_weight_n(1, p_hat_n_at_n, num, p_hat_n_at_c);

    Float weight = Reservoir::safe_weight(mi, other_rsv.sample.p_hat, other_rsv.W);
    (*output_rsv)->update(sampler->next_1d(), other_rsv.sample, weight, other_rsv.C);

    Float canonical_weight = MIS_weight_n(1, p_hat_c_at_c, num, p_hat_c_at_n) / num;
    canonical_weight = zero_if_nan(canonical_weight);

    return canonical_weight;
}

void ReSTIRDI::canonical_pairwise_MIS(const DIReservoir &canonical_rsv, Float canonical_weight,
                                      DIReservoir *output_rsv) const noexcept {
    Float weight = Reservoir::safe_weight(canonical_weight, canonical_rsv.sample.p_hat, canonical_rsv.W);
    (*output_rsv)->update(sampler()->next_1d(), canonical_rsv.sample, weight, canonical_rsv.C);
}

DIReservoir ReSTIRDI::pairwise_combine(const DIReservoir &canonical_rsv,
                                       const Container<ocarina::uint> &rsv_idx) const noexcept {
    const SampledWavelengths &swl = sampled_wavelengths();
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    SurfaceDataVar cur_surf = cur_surfaces().read(dispatch_id());
    Interaction canonical_it = pipeline()->compute_surface_interaction(cur_surf.hit, c_pos);

    DIReservoir ret;
    Float canonical_weight = 0.f;
    Uint M = rsv_idx.count() + 1;
    rsv_idx.for_each([&](const Uint &idx) {
        DIReservoir neighbor_rsv = passthrough_reservoirs().read(idx);
        SurfaceDataVar surf = cur_surfaces().read(idx);
        Interaction neighbor_it = pipeline()->compute_surface_interaction(surf.hit, c_pos);
        canonical_weight += neighbor_pairwise_MIS(canonical_rsv, canonical_it, neighbor_rsv, neighbor_it, M, &ret);
    });
    canonical_weight = ocarina::select(canonical_weight == 0.f, 1.f, canonical_weight);
    canonical_pairwise_MIS(canonical_rsv, canonical_weight, &ret);

    ret->update_W(ret.sample.p_hat);
    return ret;
}

DIReservoir ReSTIRDI::constant_combine(const DIReservoir &canonical_rsv,
                                       const Container<ocarina::uint> &rsv_idx) const noexcept {
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    SurfaceDataVar cur_surf = cur_surfaces().read(dispatch_id());
    Interaction canonical_it = pipeline()->compute_surface_interaction(cur_surf.hit, c_pos);

    DIReservoir ret;
    Uint sample_num = rsv_idx.count() + 1;
    Float cur_weight = Reservoir::cal_weight(1.f / sample_num,
                                             canonical_rsv.sample.p_hat, canonical_rsv.W);
    ret->update(0.5f, canonical_rsv.sample, cur_weight, canonical_rsv.C);

    rsv_idx.for_each([&](const Uint &idx) {
        DIReservoir rsv = passthrough_reservoirs().read(idx);
        rsv.sample.p_hat = compute_p_hat(canonical_it, nullptr, rsv.sample);
        Float neighbor_weight = Reservoir::cal_weight(1.f / sample_num,
                                                      rsv.sample.p_hat, rsv.W);
        ret->update(sampler()->next_1d(), rsv.sample, neighbor_weight, rsv.C);
    });

    ret->update_W(ret.sample.p_hat);
    return ret;
}

DIReservoir ReSTIRDI::combine_spatial(DIReservoir cur_rsv,
                                      const Container<uint> &rsv_idx) const noexcept {
    DIReservoir ret;

    if (_pairwise) {
        ret = pairwise_combine(cur_rsv, rsv_idx);
    } else {
        ret = constant_combine(cur_rsv, rsv_idx);
    }

    if (_reweight) {
    }

    return ret;
}

DIReservoir ReSTIRDI::combine_temporal(const DIReservoir &cur_rsv,
                                       SurfaceDataVar cur_surf,
                                       const DIReservoir &other_rsv) const noexcept {
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    Float3 prev_c_pos = camera->prev_device_position();
    const Geometry &geom = pipeline()->geometry();
    Interaction it = geom.compute_surface_interaction(cur_surf.hit, c_pos);

    Float p_hat_c_at_n;
    Float p_hat_n_at_n;

    Float p_hat_c_at_c;
    Float p_hat_n_at_c;

    Float mis_cur;
    Float mis_prev;

    if (_temporal.mis) {
        it.update_wo(prev_c_pos);
        p_hat_c_at_n = compute_p_hat(it, nullptr, cur_rsv.sample);
        p_hat_n_at_n = compute_p_hat(it, nullptr, other_rsv.sample);

        it.update_wo(c_pos);
        p_hat_c_at_c = compute_p_hat(it, nullptr, cur_rsv.sample);
        p_hat_n_at_c = compute_p_hat(it, nullptr, other_rsv.sample);

        mis_cur = MIS_weight_n(cur_rsv.C, p_hat_c_at_c, other_rsv.C, p_hat_c_at_n);
        mis_prev = MIS_weight_n(other_rsv.C, p_hat_n_at_n, cur_rsv.C, p_hat_n_at_c);
    } else {
        it.update_wo(prev_c_pos);
        p_hat_n_at_n = compute_p_hat(it, nullptr, other_rsv.sample);

        mis_cur = MIS_weight(cur_rsv.C, other_rsv.C);
        mis_prev = MIS_weight(other_rsv.C, cur_rsv.C);
        it.update_wo(c_pos);
    }

    DIReservoir ret;
    Float cur_weight = Reservoir::safe_weight(mis_cur,
                                              cur_rsv.sample.p_hat, cur_rsv.W);
    ret->update(0.5f, cur_rsv.sample, cur_weight, cur_rsv.C);

    auto other_sample = other_rsv.sample;
    other_sample.p_hat = p_hat_n_at_n;
    Float other_weight = Reservoir::safe_weight(mis_prev,
                                                other_sample.p_hat, other_rsv.W);

    ret->update(sampler()->next_1d(), other_sample, other_weight, other_rsv.C);
    ret->update_W(ret.sample.p_hat);
    return ret;
}

DIReservoir ReSTIRDI::temporal_reuse(DIReservoir rsv, const SurfaceDataVar &cur_surf,
                                     const Float2 &motion_vec,
                                     const SensorSample &ss,
                                     const Var<Param> &param) const noexcept {
    Float2 prev_p_film = ss.p_film - motion_vec;
    Float limit = rsv.C * param.history_limit;
    int2 res = make_int2(pipeline()->resolution());

    auto get_prev_data = [this, &limit](const Float2 &pos) {
        Uint index = dispatch_id(make_uint2(pos));
        DIReservoir prev_rsv = prev_reservoirs().read(index);
        prev_rsv->truncation(limit);
        return make_pair(prev_surfaces().read(index), prev_rsv);
    };

    $if(in_screen(make_int2(prev_p_film), res) && param.temporal) {
        auto [prev_surf, prev_rsv] = get_prev_data(prev_p_film);
        $if(is_temporal_valid(cur_surf, prev_surf, param)) {
            rsv = combine_temporal(rsv, cur_surf, prev_rsv);
        }
        $else {
            $for(i, 9) {
                Float2 p = square_to_disk(sampler()->next_2d()) * param.t_radius + prev_p_film;
                auto [another_surf, another_rsv] = get_prev_data(p);
                $if(is_temporal_valid(cur_surf, another_surf, param)) {
                    rsv = combine_temporal(rsv, cur_surf, another_rsv);
                    $break;
                };
            };
        };
    };
    return rsv;
}

void ReSTIRDI::compile_shader0() noexcept {
    Pipeline *rp = pipeline();
    const Geometry &geometry = rp->geometry();
    Camera *camera = scene().camera().get();
    Spectrum &spectrum = rp->spectrum();

    Kernel kernel = [&](Uint frame_index, Var<direct::Param> param) {
        Uint2 pixel = dispatch_idx().xy();
        camera->load_data();
        sampler()->start(pixel, frame_index, 0);
        initial(sampler(), frame_index, spectrum);
        SensorSample ss = sampler()->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);
        Var hit = geometry.trace_closest(rs.ray);

        SurfaceDataVar cur_surf;
        cur_surf.hit = hit;
        cur_surf->set_depth(0.f);
        Interaction it{false};
        $if(hit->is_hit()) {
            it = geometry.compute_surface_interaction(hit, rs.ray, true);
            cur_surf.mat_id = it.material_id();
            cur_surf->set_depth(camera->linear_depth(it.pos));
            cur_surf->set_normal(it.shading.normal());
        };
        DIReservoir rsv = RIS(hit->is_hit(), it, param);
        Float2 motion_vec = FrameBuffer::compute_motion_vec(scene().camera().get(), ss.p_film,
                                                            it.pos, hit->is_hit());
        frame_buffer().motion_vectors().write(dispatch_id(), motion_vec);

        $if(hit->is_hit()) {
            Bool occluded = geometry.occluded(it, rsv.sample->p_light());
            rsv->process_occluded(occluded);
        };
        rsv = temporal_reuse(rsv, cur_surf, motion_vec, ss, param);
        passthrough_reservoirs().write(dispatch_id(), rsv);
        cur_surfaces().write(dispatch_id(), cur_surf);
    };
    _shader0 = device().compile(kernel, "ReSTIR direct initial candidates and temporal reuse");
}

DIReservoir ReSTIRDI::spatial_reuse(DIReservoir rsv, const SurfaceDataVar &cur_surf,
                                    const Int2 &pixel, const Var<Param> &param) const noexcept {
    $if(param.spatial) {
        int2 res = make_int2(pipeline()->resolution());
        Container<uint> rsv_idx{_spatial.sample_num};
        $for(i, _spatial.sample_num) {
            Float2 offset = square_to_disk(sampler()->next_2d()) * param.s_radius;
            Int2 offset_i = make_int2(ocarina::round(offset));
            Int2 another_pixel = pixel + offset_i;
            another_pixel = ocarina::clamp(another_pixel, make_int2(0u), res - 1);
            Uint index = dispatch_id(another_pixel);
            SurfaceDataVar other_surf = cur_surfaces().read(index);
            $if(is_neighbor(cur_surf, other_surf, param)) {
                rsv_idx.push_back(index);
            };
        };
        $if(cur_surf.hit->is_hit()) {
            rsv = combine_spatial(rsv, rsv_idx);
        };
    };
    return rsv;
}

Float3 ReSTIRDI::shading(vision::DIReservoir rsv, const HitVar &hit) const noexcept {
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = pipeline()->spectrum();
    const Camera *camera = scene().camera().get();
    const Geometry &geometry = pipeline()->geometry();
    Float3 c_pos = camera->device_position();
    const SampledWavelengths &swl = sampled_wavelengths();
    SampledSpectrum value = {swl.dimension(), 0.f};
    SampledSpectrum Le = {swl.dimension(), 0.f};
    Interaction it = geometry.compute_surface_interaction(hit, c_pos);

    $if(it.has_emission()) {
        light_sampler->dispatch_light(it.light_id(), [&](const Light *light) {
            if (!light->match(LightType::Area)) { return; }
            LightSampleContext p_ref;
            p_ref.pos = camera->device_position();
            LightEval le = light->evaluate_wi(p_ref, it, swl, LightEvalMode::L);
            Le = le.L;
        });
    }
    $else {
        Interaction next_it{false};
        BSDFSample bs{swl.dimension()};
        scene().materials().dispatch(it.material_id(), [&](const Material *material) {
            auto bsdf = material->create_evaluator(it, swl);
            bs = bsdf.sample(it.wo, sampler());
        });
        RayVar ray = it.spawn_ray(bs.wi);
        HitVar hit = geometry.trace_closest(ray);
        $if(hit->is_hit()) {
            next_it = geometry.compute_surface_interaction(hit, ray, true);
            $if(next_it.has_emission()) {
                LightSampleContext p_ref;
                p_ref.pos = ray->origin();
                p_ref.ng = it.ng;
                LightEval eval = light_sampler->evaluate_hit_wi(p_ref, next_it, swl, LightEvalMode::L);
                value = eval.L * bs.eval.f / bs.eval.pdf;
            };
        };
        it.wo = normalize(camera->device_position() - it.pos);
        value = Li(it, nullptr, rsv.sample);
        Bool occluded = geometry.occluded(it, rsv.sample->p_light());
        rsv->process_occluded(occluded);
        value = value * rsv.W;
    };

    return spectrum.linear_srgb(value + Le, swl);
}

void ReSTIRDI::compile_shader1() noexcept {
    Camera *camera = scene().camera().get();
    Film *film = camera->film();
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = pipeline()->spectrum();
    Kernel kernel = [&](Uint frame_index, Var<direct::Param> param) {
        initial(sampler(), frame_index, spectrum);
        Uint2 pixel = dispatch_idx().xy();
        camera->load_data();
        sampler()->start(pixel, frame_index, 0);
        const SampledWavelengths &swl = sampled_wavelengths();
        SensorSample ss = sampler()->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);
        sampler()->start(pixel, frame_index, 1);
        SurfaceDataVar cur_surf = cur_surfaces().read(dispatch_id());
        DIReservoir temporal_rsv = passthrough_reservoirs().read(dispatch_id());
        DIReservoir st_rsv = spatial_reuse(temporal_rsv, cur_surf, make_int2(pixel), param);
        Var hit = cur_surf.hit;
        Float3 L = make_float3(0.f);
        $if(hit->is_hit()) {
            L = shading(st_rsv, hit);
        }
        $else {
            if (light_sampler->env_light()) {
                LightSampleContext p_ref;
                p_ref.pos = rs.origin();
                p_ref.ng = rs.direction();
                LightEval eval = light_sampler->evaluate_miss_wi(p_ref, rs.direction(), swl, LightEvalMode::L);
                L = spectrum.linear_srgb(eval.L, swl);
            }
        };
        frame_buffer().bufferA().write(dispatch_id(), make_float4(L, 1.f));
        cur_reservoirs().write(dispatch_id(), st_rsv);
    };
    _shader1 = device().compile(kernel, "ReSTIR direct spatial reuse and shading");
}

void ReSTIRDI::prepare() noexcept {
    using direct::Reservoir;
    Pipeline *rp = pipeline();
    _reservoirs.super() = device().create_buffer<Reservoir>(rp->pixel_num() * 3,
                                                            "ReSTIRDI::_reservoirs x 3");
    _reservoirs.register_self(0, rp->pixel_num());
    _reservoirs.register_view(rp->pixel_num(), rp->pixel_num());
    _reservoirs.register_view(rp->pixel_num() * 2, rp->pixel_num());
    vector<Reservoir> host{rp->pixel_num() * 3, Reservoir{}};
    _reservoirs.upload_immediately(host.data());
}

direct::Param ReSTIRDI::construct_param() const noexcept {
    direct::Param param;
    param.M_light = M_light;
    param.M_bsdf = M_bsdf;

    param.spatial = static_cast<uint>(_spatial.open);
    param.N = _spatial.sample_num;
    param.s_dot = _spatial.dot_threshold();
    param.s_depth = _spatial.depth_threshold;
    param.s_radius = _spatial.sampling_radius;

    param.temporal = static_cast<uint>(_temporal.open);
    param.history_limit = _temporal.limit;
    param.t_dot = _temporal.dot_threshold();
    param.t_depth = _temporal.depth_threshold;
    param.t_radius = _temporal.sampling_radius;
    return param;
}

CommandList ReSTIRDI::estimate(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    auto param = construct_param();
    ret << _shader0(frame_index, param).dispatch(rp->resolution());
    if (_open) {
        ret << _shader1(frame_index, param).dispatch(rp->resolution());
    }
    return ret;
}

}// namespace vision