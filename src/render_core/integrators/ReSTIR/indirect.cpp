//
// Created by Zero on 2023/10/29.
//

#include "indirect.h"
#include "base/integrator.h"

namespace vision {
ReSTIRGI::ReSTIRGI(IlluminationIntegrator *integrator,
                   const vision::ParameterSet &desc)
    : ReSTIR(integrator, desc) {
}

Float ReSTIRGI::Jacobian_det(Float3 cur_pos, Float3 neighbor_pos,
                             Var<SurfacePoint> sample_point) const noexcept {
    Float ret;
    Float3 cur_vec = cur_pos - sample_point->position();
    Float3 neighbor_vec = neighbor_pos - sample_point->position();
    Float cos_phi_c = abs_dot(normalize(cur_vec), sample_point->normal());
    Float cos_phi_n = abs_dot(normalize(neighbor_vec), sample_point->normal());
    Float cur_dist2 = length_squared(cur_vec);
    Float neighbor_dist2 = length_squared(neighbor_vec);
    ret = (cos_phi_c * neighbor_dist2) / (cos_phi_n * cur_dist2);
    ret = ocarina::zero_if_nan_inf(ret);
    float lower = 0.6f;
    ret = ocarina::clamp(ret, lower, ocarina::rcp(lower));
    return ret;
}

bool ReSTIRGI::render_UI(ocarina::Widgets *widgets) noexcept {
    return widgets->use_tree("ReSTIR GI", [&] {
        changed_ |= widgets->check_box("switch", &open_);
        if (open_) {
            render_sub_UI(widgets);
        }
    });
}

void ReSTIRGI::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    changed_ |= widgets->check_box("temporal", &temporal_.open);
    changed_ |= widgets->drag_uint("max age", &max_age_, 1, 0, 100);
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

GISampleVar ReSTIRGI::init_sample(const Interaction &it, const SensorSample &ss,
                                  HitBSDFVar &hit_bsdf) noexcept {
    Uint2 pixel = dispatch_idx().xy();
    sampler()->start(pixel, frame_index(), 3);
    Interaction sp_it{false};
    RayVar ray = it.spawn_ray(hit_bsdf.wi.as_vec3());
    RayState ray_state = RayState::create(ray);
    Float3 throughput = hit_bsdf->safe_throughput();
    GISampleVar sample;
    Float3 L = integrator_->Li(ray_state, hit_bsdf.pdf,
                               SampledSpectrum(throughput),
                               sp_it, *this) /
               throughput;
    L = ocarina::zero_if_nan_inf(L);
    sample.sp->set(sp_it);
    sample.Lo.set(L);
    return sample;
}

void ReSTIRGI::compile_initial_samples() noexcept {
    TSpectrum &spectrum = pipeline()->spectrum();
    TSensor &camera = scene().sensor();
    Kernel kernel = [&](Uint frame_index) {
        initial(sampler(), frame_index, spectrum);
        SurfaceDataVar surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            radiance_->write(dispatch_id(), make_float4(0.f));
            $return();
        };
        camera->load_data();
        sampler()->load_data();
        integrator_->load_data();
        Uint2 pixel = dispatch_idx().xy();
        sampler()->start(pixel, frame_index, 0);
        SensorSample ss = sampler()->sensor_sample(pixel, camera->filter());
        Float3 view_pos = cur_view_pos(surf.is_replaced);
        Interaction it = pipeline()->compute_surface_interaction(surf.hit, view_pos);
        HitBSDFVar hit_bsdf = frame_buffer().hit_bsdfs().read(dispatch_id());
        GISampleVar sample = init_sample(it, ss, hit_bsdf);
        Float3 throughput = make_float3(1.f);
        $if(surf.is_replaced) {
            throughput = cur_surface_extends().read(dispatch_id()).throughput;
        };
        sample.Lo.set(sample.Lo.as_vec3() * throughput);
        samples_.write(dispatch_id(), sample);
    };
    initial_samples_ = device().compile(kernel, "ReSTIR indirect initial samples");
}

ScatterEval ReSTIRGI::eval_bsdf(const Interaction &it, const GISampleVar &sample,
                                MaterialEvalMode mode) const noexcept {
    return outline("ReSTIRGI::eval_bsdf", [&] {
        ScatterEval ret{spectrum()->dimension(), 1};
        scene().materials().dispatch(it.material_id(), [&](const Material *material) {
            MaterialEvaluator bsdf = material->create_evaluator(it, sampled_wavelengths());
            Float3 wi = normalize(sample.sp->position() - it.pos);
            ret = bsdf.evaluate(it.wo, wi, mode);
        });
        return ret;
    });
}

Float ReSTIRGI::compute_p_hat(const vision::Interaction &it,
                              const vision::GISampleVar &sample) const noexcept {
    Float3 bsdf = eval_bsdf(it, sample, MaterialEvalMode::F).f.vec3();
    return sample->p_hat(bsdf);
}

GIReservoirVar ReSTIRGI::combine_temporal(const GIReservoirVar &cur_rsv, SurfaceDataVar cur_surf,
                                          GIReservoirVar &other_rsv, SurfaceDataVar *neighbor_surf,
                                          Float3 view_pos, Float3 prev_view_pos) const noexcept {
    other_rsv.sample.age += 1;
    TSensor &camera = scene().sensor();
    Interaction it = pipeline()->compute_surface_interaction(cur_surf.hit, view_pos);
    GIReservoirVar ret;
    Float cur_p_hat = compute_p_hat(it, cur_rsv.sample);
    ret->update(sampler()->next_1d(), cur_rsv.sample, GIReservoir::safe_weight(cur_rsv.C, cur_p_hat, cur_rsv.W));
    Float other_p_hat = compute_p_hat(it, other_rsv.sample);
    ret->update(sampler()->next_1d(), other_rsv.sample, GIReservoir::safe_weight(other_rsv.C, other_p_hat, other_rsv.W), other_rsv.C);
    Float p_hat = compute_p_hat(it, ret.sample);

    if (neighbor_surf) {
        Interaction neighbor_it = pipeline()->compute_surface_interaction(neighbor_surf->hit, prev_view_pos);
        p_hat = p_hat * Jacobian_det(it.pos, neighbor_it.pos, other_rsv.sample.sp);
    }
    ret->update_W(p_hat);
    return ret;
}

GIReservoirVar ReSTIRGI::temporal_reuse(GIReservoirVar rsv, const SurfaceDataVar &cur_surf,
                                        const Float2 &motion_vec, const SensorSample &ss,
                                        const Var<GIParam> &param) const noexcept {
    Float2 prev_p_film = ss.p_film - motion_vec;
    Float limit = rsv.C * param.history_limit;
    Int2 res = make_int2(dispatch_dim().xy());
    TSensor &camera = scene().sensor();

    Float3 view_pos = cur_view_pos(cur_surf.is_replaced);
    Float3 prev_view_pos = camera->prev_device_position();

    auto get_prev_data = [this, &limit](const Float2 &pos,
                                        Float3 &view_pos) {
        Uint index = dispatch_id(make_uint2(pos));
        GIReservoirVar prev_rsv = prev_reservoirs().read(index);
        prev_rsv->truncation(limit);
        SurfaceDataVar surf = prev_surfaces().read(index);
        $if(surf.is_replaced) {
            view_pos = prev_surface_extends().read(index).view_pos;
        };
        return make_pair(surf, prev_rsv);
    };

    $if(in_screen(make_int2(prev_p_film), res) && param.temporal) {

        auto data = get_prev_data(prev_p_film, prev_view_pos);
        auto prev_surf = data.first;
        auto prev_rsv = data.second;

        $if(is_temporal_valid(cur_surf, prev_surf,
                              param, addressof(prev_rsv.sample))) {
            rsv = combine_temporal(rsv, cur_surf, prev_rsv, nullptr,
                                   view_pos, prev_view_pos);
        }
        $else {
            $for(i, temporal_.N) {
                Float2 p = square_to_disk(sampler()->next_2d()) * param.t_radius + prev_p_film;
                auto data = get_prev_data(p, prev_view_pos);
                auto another_surf = data.first;
                auto another_rsv = data.second;
                $if(is_temporal_valid(cur_surf, another_surf,
                                      param, addressof(another_rsv.sample))) {
                    rsv = combine_temporal(rsv, cur_surf, another_rsv, addressof(another_surf),
                                           view_pos, prev_view_pos);
                    $break;
                };
            };
        };
    };
    return rsv;
}

void ReSTIRGI::compile_temporal_reuse() noexcept {
    TSpectrum &spectrum = pipeline()->spectrum();
    TSensor &camera = scene().sensor();
    //todo remedy init samples and reservoir
    Kernel kernel = [&](Var<GIParam> param, Uint frame_index) {
        initial(sampler(), frame_index, spectrum);
        SurfaceDataVar surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            $return();
        };
        sampler()->load_data();
        camera->load_data();
        Uint2 pixel = dispatch_idx().xy();
        SensorSample ss;
        sampler()->temporary([&](Sampler *sampler) {
            sampler->start(pixel, frame_index, 0);
            ss = sampler->sensor_sample(pixel, camera->filter());
        });
        sampler()->start(pixel, frame_index, 4);
        GISampleVar sample = samples_.read(dispatch_id());
        HitBSDFVar hit_bsdf = frame_buffer().hit_bsdfs().read(dispatch_id());
        GIReservoirVar rsv;
        Float p_hat = sample->p_hat(hit_bsdf.bsdf.as_vec3());
        Float weight = GIReservoir::safe_weight(1, p_hat, 1.f / hit_bsdf.pdf);
        rsv->update(0.5f, sample, weight);
        rsv->update_W(p_hat);
        Float2 motion_vec = frame_buffer().motion_vectors().read(dispatch_id());
        rsv = temporal_reuse(rsv, surf, motion_vec, ss, param);
        passthrough_reservoirs().write(dispatch_id(), rsv);
    };
    temporal_pass_ = device().compile(kernel, "ReSTIR indirect temporal reuse");
}

GIReservoirVar ReSTIRGI::constant_combine(const GIReservoirVar &canonical_rsv,
                                          const Container<uint> &rsv_idx) const noexcept {
    TSensor &camera = scene().sensor();
    SurfaceDataVar cur_surf = cur_surfaces().read(dispatch_id());
    Float3 view_pos = cur_view_pos(cur_surf.is_replaced);
    Interaction canonical_it = pipeline()->compute_surface_interaction(cur_surf.hit, view_pos);

    GIReservoirVar ret = canonical_rsv;
    Uint sample_num = rsv_idx.count() + 1;

    rsv_idx.for_each([&](const Uint &idx) {
        GIReservoirVar rsv = passthrough_reservoirs().read(idx);
        $if(luminance(rsv.sample.Lo.as_vec3()) > 0) {
            SurfaceDataVar neighbor_surf = cur_surfaces().read(idx);
            Interaction neighbor_it = pipeline()->compute_surface_interaction(neighbor_surf.hit, view_pos);
            Float p_hat = compute_p_hat(canonical_it, rsv.sample);
            p_hat = p_hat * Jacobian_det(canonical_it.pos, neighbor_it.pos, rsv.sample.sp);
            Float v = pipeline()->visibility(canonical_it, rsv.sample.sp->position());
            Float weight = GIReservoir::safe_weight(rsv.C, p_hat, rsv.W);
            ret->update(sampler()->next_1d(), rsv.sample, weight * v, rsv.C * v);
        };
    });
    Float p_hat = compute_p_hat(canonical_it, ret.sample);
    ret->update_W(p_hat);
    return ret;
}

GIReservoirVar ReSTIRGI::combine_spatial(GIReservoirVar cur_rsv,
                                         const Container<uint> &rsv_idx) const noexcept {

    cur_rsv = constant_combine(cur_rsv, rsv_idx);

    return cur_rsv;
}

GIReservoirVar ReSTIRGI::spatial_reuse(GIReservoirVar rsv, const SurfaceDataVar &cur_surf,
                                       const Int2 &pixel, const Var<GIParam> &param) const noexcept {
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
        $if(cur_surf.hit->is_hit()) {
            rsv = combine_spatial(rsv, rsv_idx);
        };
    };
    return rsv;
}

Float3 ReSTIRGI::shading(GIReservoirVar rsv,
                         const SurfaceDataVar &cur_surf) const noexcept {
    TSensor &camera = scene().sensor();
    Float3 view_pos = cur_view_pos(cur_surf.is_replaced);
    Float3 throughput = make_float3(1.f);
    $if(cur_surf.is_replaced) {
        SurfaceExtendVar surf_ext = cur_surface_extends().read(dispatch_id());
        throughput = surf_ext.throughput;
        view_pos = surf_ext.view_pos;
    };
    Interaction it = pipeline()->compute_surface_interaction(cur_surf.hit, view_pos);
    ScatterEval scatter_eval = eval_bsdf(it, rsv.sample, MaterialEvalMode::F);
    return rsv.sample.Lo.as_vec3() * scatter_eval.f.vec3() * rsv.W;
}

void ReSTIRGI::compile_spatial_shading() noexcept {
    TSensor &camera = scene().sensor();
    RadianceCollector *rad_collector = camera->rad_collector();
    TLightSampler &light_sampler = scene().light_sampler();
    TSpectrum &spectrum = pipeline()->spectrum();

    Kernel kernel = [&](Var<GIParam> param, Uint frame_index) {
        initial(sampler(), frame_index, spectrum);
        SurfaceDataVar surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            $return();
        };
        sampler()->load_data();
        sampler()->start(dispatch_idx().xy(), frame_index, 5);
        camera->load_data();
        GIReservoirVar rsv = passthrough_reservoirs().read(dispatch_id());
        rsv = spatial_reuse(rsv, surf, make_int2(dispatch_idx().xy()), param);
        Float3 L = shading(rsv, surf);
        radiance_->write(dispatch_id(), make_float4(L, 1.f));
        cur_reservoirs().write(dispatch_id(), rsv);
    };
    spatial_shading_ = device().compile(kernel, "ReSTIR indirect spatial reuse and shading");
}

GIParam ReSTIRGI::construct_param() const noexcept {
    GIParam param;
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

CommandList ReSTIRGI::dispatch(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    GIParam param = construct_param();
    ret << initial_samples_(frame_index).dispatch(rp->resolution());
    ret << temporal_pass_(param, frame_index).dispatch(rp->resolution());
    ret << spatial_shading_(param, frame_index).dispatch(rp->resolution());
    return ret;
}

void ReSTIRGI::update_resolution(ocarina::uint2 res) noexcept {
    Pipeline *rp = pipeline();
    reservoirs_.super() = device().create_buffer<GIReservoir>(rp->pixel_num() * 3,
                                                              "ReSTIRGI::reservoirs_ x 3");
    reservoirs_.register_self(0, rp->pixel_num());
    reservoirs_.register_view_index(1, rp->pixel_num(), rp->pixel_num());
    reservoirs_.register_view_index(2, rp->pixel_num() * 2, rp->pixel_num());
    vector<GIReservoir> host{rp->pixel_num() * 3, GIReservoir{}};
    reservoirs_.upload_immediately(host.data());

    samples_.super() = device().create_buffer<GISample>(rp->pixel_num(),
                                                        "ReSTIRGI::samples_");
    samples_.register_self();
    vector<GISample> vec{rp->pixel_num(), GISample{}};
    samples_.upload_immediately(vec.data());
}

void ReSTIRGI::prepare() noexcept {
    Pipeline *rp = pipeline();

    frame_buffer().prepare_screen_buffer(radiance_);

    reservoirs_.super() = device().create_buffer<GIReservoir>(rp->pixel_num() * 3,
                                                              "ReSTIRGI::reservoirs_ x 3");
    reservoirs_.register_self(0, rp->pixel_num());
    reservoirs_.register_view(rp->pixel_num(), rp->pixel_num());
    reservoirs_.register_view(rp->pixel_num() * 2, rp->pixel_num());
    vector<GIReservoir> host{rp->pixel_num() * 3, GIReservoir{}};
    reservoirs_.upload_immediately(host.data());

    samples_.super() = device().create_buffer<GISample>(rp->pixel_num(),
                                                        "ReSTIRGI::samples_");
    samples_.register_self();
    vector<GISample> vec{rp->pixel_num(), GISample{}};
    samples_.upload_immediately(vec.data());
}
}// namespace vision

VS_REGISTER_HOTFIX(vision, ReSTIRGI)