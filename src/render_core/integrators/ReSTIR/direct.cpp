//
// Created by Zero on 2023/9/15.
//

#include "direct.h"
#include "base/integrator.h"

namespace vision {

ReSTIRDI::ReSTIRDI(IlluminationIntegrator *integrator, const ParameterSet &desc)
    : ReSTIR(integrator, desc),
      M_light_(desc["M_light"].as_uint(10)),
      M_bsdf_(desc["M_bsdf"].as_uint(1)),
      debias_(desc["debias"].as_bool(false)),
      reweight_(desc["reweight"].as_bool(false)),
      pairwise_(desc["pairwise"].as_bool(false)),
      max_recursion_(desc["max_recursion"].as_uint(5)) {}

bool ReSTIRDI::render_UI(ocarina::Widgets *widgets) noexcept {
    bool open = widgets->use_tree("ReSTIR DI", [&] {
        changed_ |= widgets->check_box("switch", &open_);
        if (open_) {
            render_sub_UI(widgets);
        }
    });
    return open;
}

void ReSTIRDI::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    changed_ |= widgets->input_uint_limit("M light", &M_light_, 0, 100);
    changed_ |= widgets->input_uint_limit("M BSDF", &M_bsdf_, 0, 100);
    changed_ |= widgets->drag_uint("max age", &max_age_, 1, 0, 100);
    changed_ |= widgets->input_uint_limit("max recursion", &max_recursion_, 1, 100);
    changed_ |= widgets->check_box("temporal", &temporal_.open);
    if (temporal_.open) {
        changed_ |= widgets->input_uint_limit("history", &temporal_.limit, 0, 50, 1, 3);
        changed_ |= widgets->input_float_limit("temporal theta",
                                               &temporal_.theta, 0, 90, 1, 1);
        changed_ |= widgets->input_float_limit("temporal depth", &temporal_.depth_threshold,
                                               0, 1, 0.02, 0.1);
        changed_ |= widgets->input_float_limit("temporal radius", &temporal_.sampling_radius,
                                               0, 50, 1, 5);
    }
    changed_ |= widgets->check_box("spatial", &spatial_.open);
    if (spatial_.open) {
        changed_ |= widgets->input_float_limit("spatial theta",
                                               &spatial_.theta, 0, 90, 1, 1);
        changed_ |= widgets->input_float_limit("spatial depth", &spatial_.depth_threshold,
                                               0, 1, 0.02, 0.1);
        changed_ |= widgets->input_float_limit("spatial radius", &spatial_.sampling_radius,
                                               0, 50, 1, 5);
    }
}

SampledSpectrum ReSTIRDI::Li(const Interaction &it, MaterialEvaluator *bsdf, DISampleVar *sample,
                             BSDFSample *bs, Float *light_pdf_point, HitBSDFVar *hit_bsdf, Uint *flag) const noexcept {
    TLightSampler &light_sampler = scene().light_sampler();
    TSpectrum &spectrum = scene().spectrum();
    const SampledWavelengths &swl = sampled_wavelengths();
    const Geometry &geometry = pipeline()->geometry();
    SampledSpectrum f{swl.dimension()};
    TSampler &sampler = scene().sampler();
    if (!bsdf) {
        outline("ReSTIRDI::Li from bsdf", [&] {
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                MaterialEvaluator bsdf = material->create_evaluator(it, swl);
                if (flag) { *flag = bsdf.flag(); }
                swl.check_dispersive(spectrum, bsdf);
                *bs = bsdf.sample(it.wo, sampler);
            });
        });
    } else {
        outline("ReSTIRDI::Li from bsdf", [&] {
            *bs = bsdf->sample(it.wo, sampler);
            if (flag) { *flag = bsdf->flag(); }
        });
    }
    RayVar ray = it.spawn_ray(bs->wi);
    TriangleHitVar hit = geometry.trace_closest(ray);
    Float pdf = bs->eval.pdf();
    hit_bsdf->wi.set(bs->wi);
    hit_bsdf->bsdf.set(bs->eval.f.vec3());
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
            le = light_sampler->evaluate_hit_point(it, next_it, bs->eval.pdf(),
                                                   swl, light_pdf_point);
            lsp.light_index = light_sampler->extract_light_index(next_it);
            lsp.prim_id = hit.prim_id;
            lsp.bary = hit.bary;
            (*sample)->set_lsp(lsp);
            (*sample)->set_pos(next_it.robust_position(it.pos - next_it.pos));
            bs->eval.pdfs = le.pdf;
            f = bs->eval.f * le.L;
        };
    }
    $else {
        if (light_sampler->env_light()) {
            LightSampleContext p_ref{it};
            Float3 wi = bs->wi * scene().world_diameter();
            le = light_sampler->evaluate_miss_point(p_ref, wi, bs->eval.pdf(),
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

SampledSpectrum ReSTIRDI::Li(const Interaction &it, MaterialEvaluator *bsdf, const DISampleVar &sample,
                             LightSample *output_ls, Float *bsdf_pdf_point) const noexcept {
    TLightSampler &light_sampler = scene().light_sampler();
    TSpectrum &spectrum = scene().spectrum();
    const SampledWavelengths &swl = sampled_wavelengths();
    SampledSpectrum f{swl.dimension()};
    LightSample ls{swl.dimension()};
    $if(sample->valid()) {
        ls = light_sampler->evaluate_point(it, sample->lsp(), swl);
    };
    Float3 wi = normalize(ls.p_light - it.pos);
    ScatterEval eval{swl.dimension(), 1};
    if (!bsdf) {
        outline("ReSTIRDI::Li from light", [&] {
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                MaterialEvaluator bsdf = material->create_evaluator(it, swl);
                swl.check_dispersive(spectrum, bsdf);
                eval = bsdf.evaluate(it.wo, wi);
            });
            f = eval.f * ls.eval.L;
        });
    } else {
        outline("ReSTIRDI::Li from light", [&] {
            eval = bsdf->evaluate(it.wo, wi);
            f = eval.f * ls.eval.L;
        });
    }

    if (bsdf_pdf_point) {
        *bsdf_pdf_point = light_sampler->PDF_point(it, sample->lsp(), eval.pdf());
    }
    if (output_ls) {
        *output_ls = ls;
    }
    return f;
}

DIReservoirVar ReSTIRDI::RIS(const Bool &hit, const Interaction &it, const Var<DIParam> &param,
                             const Float3 &throughput, Uint *flag) const noexcept {
    TLightSampler &light_sampler = scene().light_sampler();
    TSampler &sampler = scene().sampler();
    TSpectrum &spectrum = scene().spectrum();
    comment("RIS start");
    Uint M_light = param.M_light;
    Uint M_bsdf = param.M_bsdf;
    DIReservoirVar ret;
    const SampledWavelengths &swl = sampled_wavelengths();
    auto sample_light = [&](MaterialEvaluator *bsdf) {
        DISampleVar sample;
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
        Float weight = DIReservoir::safe_weight(mis_weight,
                                                sample.p_hat, 1.f / light_pdf) *
                       luminance(throughput);
        ret->update(sampler->next_1d(), sample, weight);
    };

    HitBSDFVar hit_bsdf;

    auto sample_bsdf = [&](MaterialEvaluator *bsdf) {
        DISampleVar sample;
        BSDFSample bs{swl.dimension(), 1};
        Float light_pdf_point = 0.f;
        Float p_hat = compute_p_hat(it, bsdf, &sample, &bs, &light_pdf_point, &hit_bsdf, flag);
        sample.p_hat = p_hat;
        Float weight = DIReservoir::safe_weight(bs.eval.pdf() / (M_light * light_pdf_point + M_bsdf * bs.eval.pdf()),
                                                sample.p_hat, 1.f / bs.eval.pdf()) *
                       luminance(throughput);
        ret->update(sampler->next_1d(), sample, weight);
    };

    $if(hit) {
        if (integrator_->separate()) {
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

Float ReSTIRDI::neighbor_pairwise_MIS(const DIReservoirVar &canonical_rsv, const Interaction &canonical_it,
                                      const DIReservoirVar &other_rsv, const Interaction &other_it,
                                      Uint M, DIReservoirVar *output_rsv) const noexcept {
    TSampler &sampler = scene().sampler();
    const SampledWavelengths &swl = sampled_wavelengths();
    Float p_hat_c_at_c = compute_p_hat(canonical_it, nullptr, canonical_rsv.sample);
    Float p_hat_c_at_n = compute_p_hat(other_it, nullptr, canonical_rsv.sample);
    Float p_hat_n_at_n = compute_p_hat(other_it, nullptr, other_rsv.sample);
    Float p_hat_n_at_c = compute_p_hat(canonical_it, nullptr, other_rsv.sample);

    Float num = M - 1;

    Float mi = MIS_weight_n(1, p_hat_n_at_n, num, p_hat_n_at_c);

    Float weight = DIReservoir::safe_weight(mi, other_rsv.sample.p_hat, other_rsv.W);
    (*output_rsv)->update(sampler->next_1d(), other_rsv.sample, weight, other_rsv.C);

    Float canonical_weight = MIS_weight_n(1, p_hat_c_at_c, num, p_hat_c_at_n) / num;
    canonical_weight = zero_if_nan(canonical_weight);

    return canonical_weight;
}

void ReSTIRDI::canonical_pairwise_MIS(const DIReservoirVar &canonical_rsv, Float canonical_weight,
                                      DIReservoirVar *output_rsv) const noexcept {
    Float weight = DIReservoir::safe_weight(canonical_weight, canonical_rsv.sample.p_hat, canonical_rsv.W);
    (*output_rsv)->update(sampler()->next_1d(), canonical_rsv.sample, weight, canonical_rsv.C);
}

DIReservoirVar ReSTIRDI::pairwise_combine(const DIReservoirVar &canonical_rsv, Float3 view_pos,
                                          const Container<ocarina::uint> &rsv_idx) const noexcept {
    const SampledWavelengths &swl = sampled_wavelengths();
    TSensor &camera = scene().sensor();
    SurfaceDataVar cur_surf = cur_surfaces().read(dispatch_id());
    Interaction canonical_it = pipeline()->compute_surface_interaction(cur_surf.hit, view_pos);

    DIReservoirVar ret;
    Float canonical_weight = 0.f;
    Uint M = rsv_idx.count() + 1;
    rsv_idx.for_each([&](const Uint &idx) {
        DIReservoirVar neighbor_rsv = passthrough_reservoirs().read(idx);
        SurfaceDataVar surf = cur_surfaces().read(idx);
        Interaction neighbor_it = pipeline()->compute_surface_interaction(surf.hit, view_pos);
        canonical_weight += neighbor_pairwise_MIS(canonical_rsv, canonical_it, neighbor_rsv, neighbor_it, M, &ret);
    });
    canonical_weight = ocarina::select(canonical_weight == 0.f, 1.f, canonical_weight);
    canonical_pairwise_MIS(canonical_rsv, canonical_weight, &ret);

    ret->update_W(ret.sample.p_hat);
    return ret;
}

DIReservoirVar ReSTIRDI::constant_combine(const DIReservoirVar &canonical_rsv, Float3 view_pos,
                                          const Container<ocarina::uint> &rsv_idx) const noexcept {
    TSensor &camera = scene().sensor();
    SurfaceDataVar cur_surf = cur_surfaces().read(dispatch_id());
    Interaction canonical_it = pipeline()->compute_surface_interaction(cur_surf.hit, view_pos);

    DIReservoirVar ret;
    Uint sample_num = rsv_idx.count() + 1;
    Float cur_weight = DIReservoir::cal_weight(1.f / sample_num,
                                               canonical_rsv.sample.p_hat, canonical_rsv.W);
    ret->update(0.5f, canonical_rsv.sample, cur_weight, canonical_rsv.C);

    rsv_idx.for_each([&](const Uint &idx) {
        DIReservoirVar rsv = passthrough_reservoirs().read(idx);
        rsv.sample.p_hat = compute_p_hat(canonical_it, nullptr, rsv.sample);
        Float neighbor_weight = DIReservoir::cal_weight(1.f / sample_num,
                                                        rsv.sample.p_hat, rsv.W);
        ret->update(sampler()->next_1d(), rsv.sample, neighbor_weight, rsv.C);
    });

    ret->update_W(ret.sample.p_hat);
    return ret;
}

DIReservoirVar ReSTIRDI::combine_spatial(DIReservoirVar cur_rsv, Float3 view_pos,
                                         const Container<uint> &rsv_idx) const noexcept {
    DIReservoirVar ret;

    if (pairwise_) {
        ret = pairwise_combine(cur_rsv, view_pos, rsv_idx);
    } else {
        ret = constant_combine(cur_rsv, view_pos, rsv_idx);
    }

    if (reweight_) {
    }

    return ret;
}

DIReservoirVar ReSTIRDI::combine_temporal(const DIReservoirVar &cur_rsv,
                                          const SurfaceDataVar &cur_surf,
                                          DIReservoirVar &other_rsv,
                                          Float3 view_pos,
                                          Float3 prev_view_pos) const noexcept {
    other_rsv.sample.age += 1;
    TSensor &camera = scene().sensor();
    //    view_pos = camera->device_position();
    //    prev_view_pos = camera->prev_device_position();
    const Geometry &geom = pipeline()->geometry();
    Interaction it = geom.compute_surface_interaction(cur_surf.hit, view_pos);

    Float p_hat_c_at_n;
    Float p_hat_n_at_n;

    Float p_hat_c_at_c;
    Float p_hat_n_at_c;

    Float mis_cur;
    Float mis_prev;

    if (temporal_.mis) {
        it.update_wo(prev_view_pos);
        p_hat_c_at_n = compute_p_hat(it, nullptr, cur_rsv.sample);
        p_hat_n_at_n = compute_p_hat(it, nullptr, other_rsv.sample);

        it.update_wo(view_pos);
        p_hat_c_at_c = compute_p_hat(it, nullptr, cur_rsv.sample);
        p_hat_n_at_c = compute_p_hat(it, nullptr, other_rsv.sample);

        mis_cur = MIS_weight_n(cur_rsv.C, p_hat_c_at_c, other_rsv.C, p_hat_c_at_n);
        mis_prev = MIS_weight_n(other_rsv.C, p_hat_n_at_n, cur_rsv.C, p_hat_n_at_c);
    } else {
        it.update_wo(prev_view_pos);
        p_hat_n_at_n = compute_p_hat(it, nullptr, other_rsv.sample);
        mis_cur = MIS_weight(cur_rsv.C, other_rsv.C);
        mis_prev = MIS_weight(other_rsv.C, cur_rsv.C);
        it.update_wo(view_pos);
    }

    DIReservoirVar ret;
    Float cur_weight = DIReservoir::safe_weight(mis_cur,
                                                cur_rsv.sample.p_hat, cur_rsv.W);
    ret->update(0.5f, cur_rsv.sample, cur_weight, cur_rsv.C);

    auto other_sample = other_rsv.sample;
    other_sample.p_hat = p_hat_n_at_n;
    Float other_weight = DIReservoir::safe_weight(mis_prev,
                                                  other_sample.p_hat, other_rsv.W);

    ret->update(sampler()->next_1d(), other_sample, other_weight, other_rsv.C);
    ret->update_W(ret.sample.p_hat);
    return ret;
}

DIReservoirVar ReSTIRDI::temporal_reuse(DIReservoirVar rsv, const SurfaceDataVar &cur_surf,
                                        const Float2 &motion_vec,
                                        const SensorSample &ss,
                                        const Var<DIParam> &param) const noexcept {
    Float2 prev_p_film = ss.p_film - motion_vec;
    Float limit = rsv.C * param.history_limit;
    Int2 res = make_int2(dispatch_dim().xy());
    TSensor &camera = scene().sensor();

    Float3 view_pos = camera->device_position();
    Float3 prev_view_pos = camera->prev_device_position();

    auto get_prev_data = [this, &limit](const Float2 &pos, Float3 &view_pos) {
        Uint index = dispatch_id(make_uint2(pos));
        DIReservoirVar prev_rsv = prev_reservoirs().read(index);
        prev_rsv->truncation(limit);
        SurfaceDataVar surf = prev_surfaces().read(index);
        $if(surf.is_replaced) {
            view_pos = prev_surface_extends().read(index).view_pos;
        };
        return make_pair(surf, prev_rsv);
    };

    view_pos = cur_view_pos(cur_surf.is_replaced);
    $if(in_screen(make_int2(prev_p_film), res) && param.temporal) {
        auto data = get_prev_data(prev_p_film, prev_view_pos);
        auto prev_surf = data.first;
        auto prev_rsv = data.second;

        $if(is_temporal_valid(cur_surf, prev_surf, param,
                              addressof(prev_rsv.sample))) {
            rsv = combine_temporal(rsv, cur_surf, prev_rsv, view_pos, prev_view_pos);
        }
        $else {
            $for(i, temporal_.N) {
                Float2 p = square_to_disk(sampler()->next_2d()) * param.t_radius + prev_p_film;
                auto data = get_prev_data(p, prev_view_pos);
                auto another_surf = data.first;
                auto another_rsv = data.second;
                $if(is_temporal_valid(cur_surf, another_surf, param,
                                      addressof(another_rsv.sample))) {
                    rsv = combine_temporal(rsv, cur_surf, another_rsv, view_pos, prev_view_pos);
                    $break;
                };
            };
        };
    };
    return rsv;
}

SurfaceDataVar ReSTIRDI::compute_hit(RayState rs, TriangleHitVar &hit, Interaction &it,
                                     SurfaceExtendVar &surf_ext) const noexcept {
    TSensor &camera = scene().sensor();
    const Geometry &geometry = pipeline()->geometry();
    RayVar camera_ray = rs.ray;
    surf_ext.view_pos = rs.origin();

    hit = pipeline()->trace_closest(rs.ray);

    SurfaceDataVar cur_surf;
    Uint counter = 0;

    $loop {
        cur_surf.hit = hit;
        $if(!hit->is_hit()) {
            $break;
        };
        it = geometry.compute_surface_interaction(hit, rs.ray, true);
        surf_ext.t_max += rs.ray->t_max();
        Float3 v_pos = camera_ray->at(surf_ext.t_max);
        cur_surf.mat_id = it.material_id();
        Float3 w;
        scene().materials().dispatch(cur_surf.mat_id, [&](const Material *material) {
            auto bsdf = material->create_evaluator(it, sampled_wavelengths());
            cur_surf.flag = bsdf.flag();
            if (material->enable_delta()) {
                $if(cur_surf->near_specular()) {
                    BSDFSample bsdf_sample = bsdf.sample_delta(it.wo, scene().sampler());
                    w = bsdf_sample.wi;
                    surf_ext.throughput *= bsdf_sample.eval.throughput().vec3();
                };
            }
        });
        $if(counter == 0) {
            cur_surf->set_depth(camera->linear_depth(v_pos));
            cur_surf->set_normal(it.shading.normal());
            cur_surf->set_position(it.pos);
        };
        counter += 1;
        $if(counter >= max_recursion_) {
            $break;
        };
        $if(cur_surf->near_specular()) {
            surf_ext.view_pos = it.pos;
            rs = it.spawn_ray_state(w);
            hit = pipeline()->trace_closest(rs.ray);
            cur_surf.is_replaced = true;
            //            cur_surf->set_depth(0);
        }
        $else {
            $break;
        };
    };
    return cur_surf;
}

void ReSTIRDI::compile_shader0() noexcept {
    Pipeline *rp = pipeline();
    const Geometry &geometry = rp->geometry();
    TSensor &camera = scene().sensor();
    TSpectrum &spectrum = rp->spectrum();

    Kernel kernel = [&](Uint frame_index, Var<DIParam> param) {
        Uint2 pixel = dispatch_idx().xy();
        camera->load_data();
        sampler()->load_data();
        sampler()->start(pixel, frame_index, 0);
        initial(sampler(), frame_index, spectrum);
        SensorSample ss = sampler()->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);
        TriangleHitVar hit;
        Interaction it{false};
        SurfaceExtendVar surf_ext;
        SurfaceDataVar cur_surf = compute_hit(rs, hit, it, surf_ext);
        cur_surfaces().write(dispatch_id(), cur_surf);

        $if(cur_surf.is_replaced) {
            cur_surface_extends().write(dispatch_id(), surf_ext);
        };

        DIReservoirVar rsv = RIS(hit->is_hit(), it, param, surf_ext.throughput, nullptr);
        Float2 motion_vec = FrameBuffer::compute_motion_vec(scene().sensor(), ss.p_film,
                                                            rs.ray->at(surf_ext.t_max), hit->is_hit());

        frame_buffer().motion_vectors().write(dispatch_id(), motion_vec);

        $if(hit->is_hit()) {
            Bool occluded = geometry.occluded(it, rsv.sample->p_light());
            rsv->process_occluded(occluded);
        };
        rsv = temporal_reuse(rsv, cur_surf, motion_vec, ss, param);
        passthrough_reservoirs().write(dispatch_id(), rsv);
    };
    shader0_ = device().compile(kernel, "ReSTIR direct initial candidates and temporal reuse");
}

DIReservoirVar ReSTIRDI::spatial_reuse(DIReservoirVar rsv, const SurfaceDataVar &cur_surf,
                                       const Int2 &pixel, const Var<DIParam> &param) const noexcept {
    $if(param.spatial) {
        Int2 res = make_int2(dispatch_dim().xy());
        Container<uint> rsv_idx{spatial_.sample_num};
        $for(i, spatial_.sample_num) {
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
        Float3 view_pos = cur_view_pos(cur_surf.is_replaced);
        $if(cur_surf.hit->is_hit()) {
            rsv = combine_spatial(rsv, view_pos, rsv_idx);
        };
    };
    return rsv;
}

Float3 ReSTIRDI::shading(vision::DIReservoirVar rsv, const SurfaceDataVar &surf) const noexcept {
    TLightSampler &light_sampler = scene().light_sampler();
    TSpectrum &spectrum = pipeline()->spectrum();
    const TSensor &camera = scene().sensor();
    const Geometry &geometry = pipeline()->geometry();
    Float3 view_pos = camera->device_position();
    Float3 throughput = make_float3(1.f);
    $if(surf.is_replaced) {
        auto surf_ext = cur_surface_extends().read(dispatch_id());
        throughput = surf_ext.throughput;
        view_pos = surf_ext.view_pos;
    };
    const SampledWavelengths &swl = sampled_wavelengths();
    SampledSpectrum value = {swl.dimension(), 0.f};
    SampledSpectrum Le = {swl.dimension(), 0.f};
    auto hit = surf.hit;
    Interaction it = geometry.compute_surface_interaction(hit, view_pos);
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
        BSDFSample bs{swl.dimension(), 1};
        scene().materials().dispatch(it.material_id(), [&](const Material *material) {
            auto bsdf = material->create_evaluator(it, swl);
            bs = bsdf.sample(it.wo, sampler());
        });
        RayVar ray = it.spawn_ray(bs.wi);
        TriangleHitVar hit = geometry.trace_closest(ray);
        $if(hit->is_hit()) {
            next_it = geometry.compute_surface_interaction(hit, ray, true);
            $if(next_it.has_emission()) {
                LightSampleContext p_ref;
                p_ref.pos = ray->origin();
                p_ref.ng = it.ng;
                LightEval eval = light_sampler->evaluate_hit_wi(p_ref, next_it, swl, LightEvalMode::L);
                value = eval.L * bs.eval.f / bs.eval.pdf();
            };
        };

        value = Li(it, nullptr, rsv.sample) * SampledSpectrum(throughput) / luminance(throughput);
        Bool occluded = geometry.occluded(it, rsv.sample->p_light());
        rsv->process_occluded(occluded);
        value = value * rsv.W;
    };

    return spectrum->linear_srgb(value + Le, swl);
}

void ReSTIRDI::compile_shader1() noexcept {
    TSensor &camera = scene().sensor();
    Film *film = camera->film();
    TLightSampler &light_sampler = scene().light_sampler();
    TSpectrum &spectrum = pipeline()->spectrum();
    Kernel kernel = [&](Uint frame_index, Var<DIParam> param) {
        sampler()->load_data();
        initial(sampler(), frame_index, spectrum);
        Uint2 pixel = dispatch_idx().xy();
        camera->load_data();
        sampler()->start(pixel, frame_index, 0);
        const SampledWavelengths &swl = sampled_wavelengths();
        SensorSample ss = sampler()->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);
        sampler()->start(pixel, frame_index, 1);
        SurfaceDataVar cur_surf = cur_surfaces().read(dispatch_id());
        DIReservoirVar temporal_rsv = passthrough_reservoirs().read(dispatch_id());
        DIReservoirVar st_rsv = spatial_reuse(temporal_rsv, cur_surf, make_int2(pixel), param);
        Var hit = cur_surf.hit;
        Float3 L = make_float3(0.f);
        $if(hit->is_hit()) {
            L = shading(st_rsv, cur_surf);
        }
        $else {
            if (light_sampler->env_light()) {
                LightSampleContext p_ref;
                p_ref.pos = rs.origin();
                p_ref.ng = rs.direction();
                LightEval eval = light_sampler->evaluate_miss_wi(p_ref, rs.direction(), swl, LightEvalMode::L);
                L = spectrum->linear_srgb(eval.L, swl);
            }
        };
        radiance_->write(dispatch_id(), make_float4(L, 1.f));
        cur_reservoirs().write(dispatch_id(), st_rsv);
    };
    shader1_ = device().compile(kernel, "ReSTIR direct spatial reuse and shading");
}

void ReSTIRDI::prepare() noexcept {
    Pipeline *rp = pipeline();
    frame_buffer().prepare_screen_buffer(radiance_);
    reservoirs_.super() = device().create_buffer<DIReservoir>(rp->pixel_num() * 3,
                                                              "ReSTIRDI::reservoirs_ x 3");
    reservoirs_.register_self(0, rp->pixel_num());
    reservoirs_.register_view(rp->pixel_num(), rp->pixel_num());
    reservoirs_.register_view(rp->pixel_num() * 2, rp->pixel_num());
    vector<DIReservoir> host{rp->pixel_num() * 3, DIReservoir{}};
    reservoirs_.upload_immediately(host.data());
}

void ReSTIRDI::update_resolution(ocarina::uint2 res) noexcept {
    Pipeline *rp = pipeline();
    reservoirs_.super() = device().create_buffer<DIReservoir>(rp->pixel_num() * 3,
                                                              "ReSTIRDI::reservoirs_ x 3");
    reservoirs_.register_self(0, rp->pixel_num());
    reservoirs_.register_view_index(1, rp->pixel_num(), rp->pixel_num());
    reservoirs_.register_view_index(2, rp->pixel_num() * 2, rp->pixel_num());
    vector<DIReservoir> host{rp->pixel_num() * 3, DIReservoir{}};
    reservoirs_.upload_immediately(host.data());
}

DIParam ReSTIRDI::construct_param() const noexcept {
    DIParam param;
    param.M_light = M_light_;
    param.M_bsdf = M_bsdf_;
    param.max_age = max_age_;

    param.spatial = static_cast<uint>(spatial_.open);
    param.N = spatial_.sample_num;
    param.s_dot = spatial_.dot_threshold();
    param.s_depth = spatial_.depth_threshold;
    param.s_radius = spatial_.sampling_radius;

    param.temporal = static_cast<uint>(temporal_.open);
    param.history_limit = temporal_.limit;
    param.t_dot = temporal_.dot_threshold();
    param.t_depth = temporal_.depth_threshold;
    param.t_radius = temporal_.sampling_radius;
    return param;
}

CommandList ReSTIRDI::dispatch(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    auto param = construct_param();
    ret << shader0_(frame_index, param).dispatch(rp->resolution());
    if (open_) {
        ret << shader1_(frame_index, param).dispatch(rp->resolution());
    }
    return ret;
}

}// namespace vision
VS_REGISTER_HOTFIX(vision, ReSTIRDI)
VS_REGISTER_CURRENT_PATH(1, "vision-integrator-ReSTIR.dll")