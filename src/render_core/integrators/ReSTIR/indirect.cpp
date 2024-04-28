//
// Created by Zero on 2023/10/29.
//

#include "indirect.h"
#include "base/integrator.h"

namespace vision {

ReSTIRGI::ReSTIRGI(IlluminationIntegrator *integrator,
                   const vision::ParameterSet &desc)
    : integrator_(integrator),
      spatial_(desc["spatial"]),
      temporal_(desc["temporal"]),
      open_(desc["open"].as_bool(true)),
      max_age_(desc["max_age"].as_uint(30)) {
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

GIRSVSample ReSTIRGI::init_sample(const Interaction &it, const SensorSample &ss,
                                  HitBSDFVar &hit_bsdf) noexcept {
    Uint2 pixel = dispatch_idx().xy();
    sampler()->start(pixel, frame_index(), 3);
    Interaction sp_it{false};
    RayState ray_state = RayState::create(hit_bsdf.next_ray);
    Float3 throughput = hit_bsdf->safe_throughput();
    GIRSVSample sample;
    sample.vp->set(it);
    $if(hit_bsdf.next_hit->is_hit()) {
        Float3 L = integrator_->Li(ray_state, hit_bsdf.pdf,
                                   SampledSpectrum(throughput),
                                   sp_it, *this) /
                   throughput;
        L = ocarina::zero_if_nan_inf(L);
        sample.sp->set(sp_it);
        sample.Lo.set(L);
    };
    return sample;
}

void ReSTIRGI::compile_initial_samples() noexcept {
    Spectrum &spectrum = pipeline()->spectrum();
    Camera &camera = scene().camera();
    Kernel kernel = [&](Uint frame_index) {
        initial(sampler(), frame_index, spectrum);
        integrator_->load_data();
        SurfaceDataVar surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            frame_buffer().bufferB().write(dispatch_id(), make_float4(0.f));
            $return();
        };
        camera->load_data();
        Uint2 pixel = dispatch_idx().xy();
        sampler()->start(pixel, frame_index, 0);
        SensorSample ss = sampler()->sensor_sample(pixel, camera->filter());
        Interaction it = pipeline()->compute_surface_interaction(surf.hit, camera->device_position());
        HitBSDFVar hit_bsdf = frame_buffer().hit_bsdfs().read(dispatch_id());
        GIRSVSample sample = init_sample(it, ss, hit_bsdf);
        samples_.write(dispatch_id(), sample);
    };
    initial_samples_ = device().compile(kernel, "ReSTIR indirect initial samples");
}

ScatterEval ReSTIRGI::eval_bsdf(const Interaction &it, const GIRSVSample &sample,
                                MaterialEvalMode mode) const noexcept {
    return outline(
        [&] {
            ScatterEval ret{spectrum()->dimension()};
            scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                MaterialEvaluator bsdf = material->create_evaluator(it, sampled_wavelengths());
                Float3 wi = normalize(sample.sp->position() - it.pos);
                ret = bsdf.evaluate(it.wo, wi, mode);
            });
            return ret;
        },
        "ReSTIRGI::eval_bsdf");
}

Float ReSTIRGI::compute_p_hat(const vision::Interaction &it,
                              const vision::GIRSVSample &sample) const noexcept {
    return sample->p_hat(abs_dot(it.ng, normalize(sample.sp->position() - it.pos)));
}

GIReservoir ReSTIRGI::combine_temporal(const GIReservoir &cur_rsv, SurfaceDataVar cur_surf,
                                       GIReservoir &other_rsv, SurfaceDataVar *neighbor_surf) const noexcept {
    other_rsv.sample.age += 1;
    Camera &camera = scene().camera();
    GIReservoir ret = other_rsv;
    ret->update(sampler()->next_1d(), cur_rsv.sample, cur_rsv.weight_sum);
    Interaction it = pipeline()->compute_surface_interaction(cur_surf.hit, camera->device_position());
    Float p_hat = compute_p_hat(it, ret.sample);
    if (neighbor_surf) {
        Interaction neighbor_it = pipeline()->compute_surface_interaction(neighbor_surf->hit, camera->device_position());
        p_hat = p_hat * Jacobian_det(it.pos, neighbor_it.pos, other_rsv.sample.sp);
    }
    ret->update_W(p_hat);
    return ret;
}

GIReservoir ReSTIRGI::temporal_reuse(GIReservoir rsv, const SurfaceDataVar &cur_surf,
                                     const Float2 &motion_vec, const SensorSample &ss,
                                     const Var<indirect::Param> &param) const noexcept {
    Float2 prev_p_film = ss.p_film - motion_vec;
    Float limit = rsv.C * param.history_limit;
    int2 res = make_int2(pipeline()->resolution());

    auto get_prev_data = [this, &limit](const Float2 &pos) {
        Uint index = dispatch_id(make_uint2(pos));
        GIReservoir prev_rsv = prev_reservoirs().read(index);
        prev_rsv->truncation(limit);
        return make_pair(prev_surfaces().read(index), prev_rsv);
    };

    $if(in_screen(make_int2(prev_p_film), res) && param.temporal) {

        auto data = get_prev_data(prev_p_film);
        auto prev_surf = data.first;
        auto prev_rsv = data.second;

        $if(is_temporal_valid(cur_surf, prev_surf,
                              prev_rsv.sample, param)) {
            rsv = combine_temporal(rsv, cur_surf, prev_rsv);
        }
        $else {
            $for(i, temporal_.N) {
                Float2 p = square_to_disk(sampler()->next_2d()) * param.t_radius + prev_p_film;
                auto data = get_prev_data(p);
                auto another_surf = data.first;
                auto another_rsv = data.second;
                $if(is_temporal_valid(cur_surf, another_surf,
                                      another_rsv.sample, param)) {
                    rsv = combine_temporal(rsv, cur_surf, another_rsv, addressof(another_surf));
                    $break;
                };
            };
        };
    };
    return rsv;
}

void ReSTIRGI::compile_temporal_reuse() noexcept {
    Spectrum &spectrum = pipeline()->spectrum();
    Camera &camera = scene().camera();
    Kernel kernel = [&](Var<indirect::Param> param, Uint frame_index) {
        initial(sampler(), frame_index, spectrum);
        SurfaceDataVar surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            $return();
        };
        camera->load_data();
        Uint2 pixel = dispatch_idx().xy();
        SensorSample ss;
        sampler()->temporary([&](SamplerImpl *sampler) {
            sampler->start(pixel, frame_index, 0);
            ss = sampler->sensor_sample(pixel, camera->filter());
        });
        sampler()->start(pixel, frame_index, 4);
        GIRSVSample sample = samples_.read(dispatch_id());
        HitBSDFVar hit_bsdf = frame_buffer().hit_bsdfs().read(dispatch_id());
        GIReservoir rsv;
        Float p_hat = sample->p_hat(hit_bsdf.cos_theta);
        Float weight = Reservoir::safe_weight(1, p_hat, 1.f / hit_bsdf.pdf);
        rsv->update(0.5f, sample, weight);
        rsv->update_W(p_hat);
        Float2 motion_vec = frame_buffer().motion_vectors().read(dispatch_id());
        rsv = temporal_reuse(rsv, surf, motion_vec, ss, param);
        passthrough_reservoirs().write(dispatch_id(), rsv);
    };
    temporal_pass_ = device().compile(kernel, "ReSTIR indirect temporal reuse");
}

GIReservoir ReSTIRGI::constant_combine(const GIReservoir &canonical_rsv,
                                       const Container<uint> &rsv_idx) const noexcept {
    Camera &camera = scene().camera();
    Float3 c_pos = camera->device_position();
    SurfaceDataVar cur_surf = cur_surfaces().read(dispatch_id());
    Interaction canonical_it = pipeline()->compute_surface_interaction(cur_surf.hit, c_pos);

    GIReservoir ret = canonical_rsv;
    Uint sample_num = rsv_idx.count() + 1;

    rsv_idx.for_each([&](const Uint &idx) {
        GIReservoir rsv = passthrough_reservoirs().read(idx);
        $if(luminance(rsv.sample.Lo.as_vec3()) > 0) {
            SurfaceDataVar neighbor_surf = cur_surfaces().read(idx);
            Interaction neighbor_it = pipeline()->compute_surface_interaction(neighbor_surf.hit, c_pos);
            Float p_hat = compute_p_hat(canonical_it, rsv.sample);
            p_hat = p_hat * Jacobian_det(canonical_it.pos, neighbor_it.pos, rsv.sample.sp);
            Float v = pipeline()->visibility(canonical_it, rsv.sample.sp->position());
            Float weight = Reservoir::safe_weight(rsv.C, p_hat, rsv.W);
            ret->update(sampler()->next_1d(), rsv.sample, weight * v, rsv.C * v);
        };
    });
    Float p_hat = compute_p_hat(canonical_it, ret.sample);
    ret->update_W(p_hat);
    return ret;
}

GIReservoir ReSTIRGI::combine_spatial(GIReservoir cur_rsv,
                                      const Container<uint> &rsv_idx) const noexcept {

    cur_rsv = constant_combine(cur_rsv, rsv_idx);

    return cur_rsv;
}

GIReservoir ReSTIRGI::spatial_reuse(GIReservoir rsv, const SurfaceDataVar &cur_surf,
                                    const Int2 &pixel, const Var<indirect::Param> &param) const noexcept {
    $if(param.spatial) {
        int2 res = make_int2(pipeline()->resolution());
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

Float3 ReSTIRGI::shading(indirect::GIReservoir rsv,
                         const SurfaceDataVar &cur_surf) const noexcept {
    Camera &camera = scene().camera();
    Interaction it = pipeline()->compute_surface_interaction(cur_surf.hit, camera->device_position());
    ScatterEval scatter_eval = eval_bsdf(it, rsv.sample, MaterialEvalMode::F);
    return rsv.sample.Lo.as_vec3() * scatter_eval.f.vec3() * rsv.W;
}

void ReSTIRGI::compile_spatial_shading() noexcept {
    Camera &camera = scene().camera();
    Film *film = camera->film();
    LightSampler &light_sampler = scene().light_sampler();
    Spectrum &spectrum = pipeline()->spectrum();

    Kernel kernel = [&](Var<indirect::Param> param, Uint frame_index) {
        sampler()->try_load_data();
        sampler()->start(dispatch_idx().xy(), frame_index, 5);
        initial(sampler(), frame_index, spectrum);
        camera->load_data();
        SurfaceDataVar surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            $return();
        };
        GIReservoir rsv = passthrough_reservoirs().read(dispatch_id());
        rsv = spatial_reuse(rsv, surf, make_int2(dispatch_idx().xy()), param);
        Float3 L = shading(rsv, surf);
        frame_buffer().bufferB().write(dispatch_id(), make_float4(L, 1.f));
        cur_reservoirs().write(dispatch_id(), rsv);
    };
    spatial_shading_ = device().compile(kernel, "ReSTIR indirect spatial reuse and shading");
}

indirect::Param ReSTIRGI::construct_param() const noexcept {
    indirect::Param param;
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

CommandList ReSTIRGI::estimate(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    indirect::Param param = construct_param();
    ret << initial_samples_(frame_index).dispatch(rp->resolution());
    ret << temporal_pass_(param, frame_index).dispatch(rp->resolution());
    ret << spatial_shading_(param, frame_index).dispatch(rp->resolution());
    return ret;
}

void ReSTIRGI::prepare() noexcept {
    using indirect::Reservoir;
    Pipeline *rp = pipeline();

    reservoirs_.super() = device().create_buffer<Reservoir>(rp->pixel_num() * 3,
                                                            "ReSTIRGI::reservoirs_ x 3");
    reservoirs_.register_self(0, rp->pixel_num());
    reservoirs_.register_view(rp->pixel_num(), rp->pixel_num());
    reservoirs_.register_view(rp->pixel_num() * 2, rp->pixel_num());
    vector<Reservoir> host{rp->pixel_num() * 3, Reservoir{}};
    reservoirs_.upload_immediately(host.data());

    using indirect::RSVSample;
    samples_.super() = device().create_buffer<RSVSample>(rp->pixel_num(),
                                                         "ReSTIRGI::samples_");
    samples_.register_self();
    vector<RSVSample> vec{rp->pixel_num(), RSVSample{}};
    samples_.upload_immediately(vec.data());
}

}// namespace vision