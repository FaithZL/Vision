//
// Created by Zero on 2023/10/29.
//

#include "indirect.h"
#include "base/integrator.h"

namespace vision {

ReSTIRIndirectIllumination::ReSTIRIndirectIllumination(RayTracingIntegrator *integrator,
                                                       const vision::ParameterSet &desc)
    : _integrator(integrator),
      _spatial(desc["spatial"]),
      _temporal(desc["temporal"]),
      _open(desc["open"].as_bool(true)) {
}

Float ReSTIRIndirectIllumination::Jacobian_det(Float3 cur_pos, Float3 neighbor_pos,
                                               Var<SurfacePoint> sample_point) const noexcept {
    Float ret;
    Float3 cur_vec = cur_pos - sample_point->position();
    Float3 neighbor_vec = neighbor_pos - sample_point->position();
    Float cos_phi_c = abs_dot(normalize(cur_vec), sample_point->normal());
    Float cos_phi_n = abs_dot(normalize(neighbor_vec), sample_point->normal());
    Float cur_dist2 = length_squared(cur_vec);
    Float neighbor_dist2 = length_squared(neighbor_vec);
    ret = (cos_phi_n * cur_dist2) / (cos_phi_c * neighbor_dist2);
    ret = ocarina::select(sample_point.is_valid(), ret, 1.f);
    return ret;
}

IIRSVSample ReSTIRIndirectIllumination::init_sample(const Interaction &it, const SensorSample &ss,
                                                    OCHitBSDF &hit_bsdf) noexcept {
    Uint2 pixel = dispatch_idx().xy();
    sampler()->start(pixel, frame_index(), 3);
    Interaction sp_it{false};
    RayState ray_state = RayState::create(hit_bsdf.next_ray);
    Float3 throughput = hit_bsdf->safe_throughput();
    IIRSVSample sample;
    sample.vp->set(it);
    $if(hit_bsdf.next_hit->is_hit()) {
        Float3 L = _integrator->Li(ray_state, hit_bsdf.pdf, SampledSpectrum(throughput), sp_it, *this) / throughput;
        L = ocarina::zero_if_nan_inf(L);
        sample.sp->set(sp_it);
        sample.Lo.set(L);
    };
    return sample;
}

void ReSTIRIndirectIllumination::compile_initial_samples() noexcept {
    Spectrum &spectrum = pipeline()->spectrum();
    Camera *camera = scene().camera().get();

    Kernel kernel = [&](Uint frame_index) {
        initial(sampler(), frame_index, spectrum);
        OCSurfaceData surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            _integrator->radiance1().write(dispatch_id(), make_float3(0.f));
            $return();
        };
        camera->load_data();
        Uint2 pixel = dispatch_idx().xy();
        sampler()->start(pixel, frame_index, 0);
        SensorSample ss = sampler()->sensor_sample(pixel, camera->filter());
        Interaction it = pipeline()->compute_surface_interaction(surf.hit, camera->device_position());
        OCHitBSDF hit_bsdf = frame_buffer().hit_bsdfs().read(dispatch_id());
        IIRSVSample sample = init_sample(it, ss, hit_bsdf);
        _samples.write(dispatch_id(), sample);
    };

    _initial_samples = device().async_compile(ocarina::move(kernel), "indirect initial samples");
}

ScatterEval ReSTIRIndirectIllumination::eval_bsdf(const Interaction &it, const IIRSVSample &sample,
                                                  MaterialEvalMode mode) const noexcept {
    return outline([&] {
        ScatterEval ret{spectrum().dimension()};
        scene().materials().dispatch(it.material_id(), [&](const Material *material) {
            MaterialEvaluator bsdf = material->create_evaluator(it, sampled_wavelengths());
            Float3 wi = normalize(sample.sp->position() - it.pos);
            ret = bsdf.evaluate(it.wo, wi, mode);
        });
        return ret;
    },
                   "ReSTIRIndirectIllumination::eval_bsdf");
}

Float ReSTIRIndirectIllumination::compute_p_hat(const vision::Interaction &it, const vision::IIRSVSample &sample) const noexcept {
    //    Float3 bsdf = eval_bsdf(it, sample, MaterialEvalMode::F).f.vec3();
    return sample->p_hat(abs_dot(it.ng, normalize(sample.sp->position() - it.pos)));
}

IIReservoir ReSTIRIndirectIllumination::combine_temporal(const IIReservoir &cur_rsv, OCSurfaceData cur_surf,
                                                         const IIReservoir &other_rsv) const noexcept {
    Camera *camera = scene().camera().get();
    IIReservoir ret = other_rsv;
    ret->update(sampler()->next_1d(), cur_rsv.sample, cur_rsv.weight_sum);
    Interaction it = pipeline()->compute_surface_interaction(cur_surf.hit, camera->device_position());
    Float p_hat = compute_p_hat(it, ret.sample);
    ret->update_W(p_hat);
    return ret;
}

IIReservoir ReSTIRIndirectIllumination::temporal_reuse(IIReservoir rsv, const OCSurfaceData &cur_surf,
                                                       const Float2 &motion_vec, const SensorSample &ss) const noexcept {
    if (!_temporal.open) {
        return rsv;
    }
    Float2 prev_p_film = ss.p_film - motion_vec;
    Float limit = rsv.C * _temporal.limit;
    int2 res = make_int2(pipeline()->resolution());
    $if(in_screen(make_int2(prev_p_film), res)) {
        Uint index = dispatch_id(make_uint2(prev_p_film));
        IIReservoir prev_rsv = prev_reservoirs().read(index);
        prev_rsv->truncation(limit);
        OCSurfaceData another_surf = prev_surfaces().read(index);
        $if(is_temporal_valid(cur_surf, another_surf)) {
            rsv = combine_temporal(rsv, cur_surf, prev_rsv);
        };
    };
    return rsv;
}

void ReSTIRIndirectIllumination::compile_temporal_reuse() noexcept {
    Spectrum &spectrum = pipeline()->spectrum();
    Camera *camera = scene().camera().get();
    Kernel kernel = [&](Uint frame_index) {
        initial(sampler(), frame_index, spectrum);
        OCSurfaceData surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            $return();
        };
        camera->load_data();
        Uint2 pixel = dispatch_idx().xy();
        SensorSample ss;
        sampler()->temporary([&](Sampler *sampler) {
            sampler->start(pixel, frame_index, 0);
            ss = sampler->sensor_sample(pixel, camera->filter());
        });
        sampler()->start(pixel, frame_index, 4);
        IIRSVSample sample = _samples.read(dispatch_id());
        OCHitBSDF hit_bsdf = frame_buffer().hit_bsdfs().read(dispatch_id());
        IIReservoir rsv;
        Float p_hat = sample->p_hat(hit_bsdf.cos_theta);
        Float weight = Reservoir::safe_weight(1, p_hat, 1.f / hit_bsdf.pdf);
        rsv->update(0.5f, sample, weight);
        rsv->update_W(p_hat);
        Float2 motion_vec = frame_buffer().motion_vectors().read(dispatch_id());
        rsv = temporal_reuse(rsv, surf, motion_vec, ss);
        passthrough_reservoirs().write(dispatch_id(), rsv);
    };
    _temporal_pass = device().async_compile(ocarina::move(kernel), "indirect temporal reuse");
}

IIReservoir ReSTIRIndirectIllumination::constant_combine(const IIReservoir &canonical_rsv, const Container<uint> &rsv_idx) const noexcept {
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    OCSurfaceData cur_surf = cur_surfaces().read(dispatch_id());
    Interaction canonical_it = pipeline()->compute_surface_interaction(cur_surf.hit, c_pos);

    IIReservoir ret = canonical_rsv;
    Uint sample_num = rsv_idx.count() + 1;

    rsv_idx.for_each([&](const Uint &idx) {
        IIReservoir rsv = passthrough_reservoirs().read(idx);
        $if(luminance(rsv.sample.Lo.as_vec3()) > 0) {
            OCSurfaceData neighbor_surf = cur_surfaces().read(idx);
            Interaction neighbor_it = pipeline()->compute_surface_interaction(neighbor_surf.hit, c_pos);
            Float p_hat = compute_p_hat(canonical_it, rsv.sample);
            //            p_hat = p_hat / Jacobian_det(canonical_it.pos, neighbor_it.pos, rsv.sample.sp);
            Float v = pipeline()->visibility(canonical_it, rsv.sample.sp->position());
            Float weight = Reservoir::safe_weight(rsv.C, p_hat, rsv.W);
            ret->update(sampler()->next_1d(), rsv.sample, weight * v, rsv.C * v);
        };
    });

    Float p_hat = compute_p_hat(canonical_it, ret.sample);
    ret->update_W(p_hat);

    return ret;
}

IIReservoir ReSTIRIndirectIllumination::combine_spatial(IIReservoir cur_rsv,
                                                        const Container<uint> &rsv_idx) const noexcept {

    cur_rsv = constant_combine(cur_rsv, rsv_idx);

    return cur_rsv;
}

IIReservoir ReSTIRIndirectIllumination::spatial_reuse(IIReservoir rsv, const OCSurfaceData &cur_surf,
                                                      const Int2 &pixel) const noexcept {
    if (!_spatial.open) {
        return rsv;
    }
    int2 res = make_int2(pipeline()->resolution());
    Container<uint> rsv_idx{_spatial.sample_num};
    $for(i, _spatial.sample_num) {
        Float2 offset = square_to_disk(sampler()->next_2d()) * _spatial.sampling_radius;
        Int2 offset_i = make_int2(ocarina::round(offset));
        Int2 another_pixel = pixel + offset_i;
        another_pixel = ocarina::clamp(another_pixel, make_int2(0u), res - 1);
        Uint index = dispatch_id(another_pixel);
        OCSurfaceData other_surf = cur_surfaces().read(index);
        $if(is_neighbor(cur_surf, other_surf)) {
            rsv_idx.push_back(index);
        };
    };
    $if(cur_surf.hit->is_hit()) {
        rsv = combine_spatial(rsv, rsv_idx);
    };
    return rsv;
}

Float3 ReSTIRIndirectIllumination::shading(ReSTIRIndirect::IIReservoir rsv, const OCSurfaceData &cur_surf) const noexcept {
    Camera *camera = scene().camera().get();
    Interaction it = pipeline()->compute_surface_interaction(cur_surf.hit, camera->device_position());
    ScatterEval scatter_eval = eval_bsdf(it, rsv.sample, MaterialEvalMode::F);
    return rsv.sample.Lo.as_vec3() * scatter_eval.f.vec3() * rsv.W;
}

void ReSTIRIndirectIllumination::compile_spatial_shading() noexcept {
    Camera *camera = scene().camera().get();
    Film *film = camera->radiance_film();
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = pipeline()->spectrum();

    Kernel kernel = [&](Uint frame_index) {
        sampler()->try_load_data();
        sampler()->start(dispatch_idx().xy(), frame_index, 5);
        initial(sampler(), frame_index, spectrum);
        camera->load_data();
        OCSurfaceData surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            $return();
        };
        IIReservoir rsv = passthrough_reservoirs().read(dispatch_id());
        rsv = spatial_reuse(rsv, surf, make_int2(dispatch_idx().xy()));
        Float3 L = shading(rsv, surf);
        _integrator->radiance1().write(dispatch_id(), L);
        cur_reservoirs().write(dispatch_id(), rsv);
    };
    _spatial_shading = device().async_compile(ocarina::move(kernel), "indirect spatial reuse and shading");
}

CommandList ReSTIRIndirectIllumination::estimate(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    if (_open) {
        ret << _initial_samples.get()(frame_index).dispatch(rp->resolution());
        ret << _temporal_pass.get()(frame_index).dispatch(rp->resolution());
        ret << _spatial_shading.get()(frame_index).dispatch(rp->resolution());
    }
    return ret;
}

void ReSTIRIndirectIllumination::prepare() noexcept {
    using ReSTIRIndirect::Reservoir;
    Pipeline *rp = pipeline();

    _reservoirs.super() = device().create_buffer<Reservoir>(rp->pixel_num() * 3,
                                                            "ReSTIRIndirectIllumination::_reservoirs x 3");
    _reservoirs.register_self(0, rp->pixel_num());
    _reservoirs.register_view(rp->pixel_num(), rp->pixel_num());
    _reservoirs.register_view(rp->pixel_num() * 2, rp->pixel_num());
    vector<Reservoir> host{rp->pixel_num() * 3, Reservoir{}};
    _reservoirs.upload_immediately(host.data());

    using ReSTIRIndirect::RSVSample;
    _samples.super() = device().create_buffer<RSVSample>(rp->pixel_num(),
                                                         "ReSTIRIndirectIllumination::_samples");
    _samples.register_self();
    vector<RSVSample> vec{rp->pixel_num(), RSVSample{}};
    _samples.upload_immediately(vec.data());
}

}// namespace vision