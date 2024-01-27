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
    Float3 cur_vec = cur_pos - sample_point->position();
    Float3 neighbor_vec = neighbor_pos - sample_point->position();
    Float cos_phi_c = abs_dot(normalize(cur_vec), sample_point->normal());
    Float cos_phi_n = abs_dot(normalize(neighbor_vec), sample_point->normal());
    Float cur_dist2 = length_squared(cur_vec);
    Float neighbor_dist2 = length_squared(neighbor_vec);
    return (cos_phi_n * cur_dist2) / (cos_phi_c * neighbor_dist2);
}

IIReservoir ReSTIRIndirectIllumination::combine_temporal(const IIReservoir &cur_rsv, OCSurfaceData cur_surf,
                                                         const IIReservoir &other_rsv,
                                                         vision::SampledWavelengths &swl) const noexcept {
    Camera *camera = scene().camera().get();
    Float3 c_pos = camera->device_position();
    Float3 prev_c_pos = camera->prev_device_position();

    IIReservoir ret;

    return ret;
}

IIReservoir ReSTIRIndirectIllumination::temporal_reuse(IIReservoir rsv, const OCSurfaceData &cur_surf,
                                                       const Float2 &motion_vec, const SensorSample &ss,
                                                       vision::SampledWavelengths &swl) const noexcept {
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
            rsv = combine_temporal(rsv, cur_surf, prev_rsv, swl);
        };
    };
    return rsv;
}

IIRSVSample ReSTIRIndirectIllumination::init_sample(const Interaction &it, const SensorSample &ss,
                                                    const OCHitContext &hit_context,
                                                    SampledWavelengths &swl) noexcept {
    Camera *camera = scene().camera().get();
    Film *film = camera->radiance_film();
    LightSampler *light_sampler = scene().light_sampler();

    Uint2 pixel = dispatch_idx().xy();
    sampler()->start(pixel, *_frame_index, 3);
    Interaction sp_it;
    RayState ray_state{hit_context.next_ray, 1.f, InvalidUI32};
    Float3 L = _integrator->Li(ray_state, hit_context.pdf, &sp_it);
    IIRSVSample sample;
    _integrator->indirect_light().write(dispatch_id(), L * hit_context->throughput());
    sample.vp->set(it);
    sample.sp->set(sp_it);
    sample.Lo.set(L);
    return sample;
}

void ReSTIRIndirectIllumination::compile_shader0() noexcept {
    Spectrum &spectrum = pipeline()->spectrum();
    Camera *camera = scene().camera().get();
    Kernel kernel = [&](Uint frame_index) {
        _frame_index.emplace(frame_index);
        OCSurfaceData surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            _integrator->indirect_light().write(dispatch_id(), make_float3(0.f));
            $return();
        };
        camera->load_data();
        Uint2 pixel = dispatch_idx().xy();
        sampler()->start(pixel, frame_index, 0);
        SampledWavelengths swl = spectrum.sample_wavelength(sampler());
        SensorSample ss = sampler()->sensor_sample(pixel, camera->filter());
        Interaction it = pipeline()->compute_surface_interaction(surf.hit, camera->device_position());
        OCHitContext hit_context = _integrator->hit_contexts().read(dispatch_id());
        IIRSVSample sample = init_sample(it, ss, hit_context, swl);
        IIReservoir rsv;
        Float weight = Reservoir::cal_weight(1, ocarina::luminance(sample.Lo.as_vec()), 1.f/hit_context.pdf);
        rsv->update(0.5f, sample, weight);

    };
    _shader0 = async([&, kernel = ocarina::move(kernel)] {
        return device().compile(kernel, "indirect initial samples and temporal reuse");
    });
}

void ReSTIRIndirectIllumination::compile_shader1() noexcept {
    Camera *camera = scene().camera().get();
    Film *film = camera->radiance_film();
    LightSampler *light_sampler = scene().light_sampler();
    Spectrum &spectrum = pipeline()->spectrum();

    Kernel kernel = [&](Uint frame_index) {
        _frame_index.emplace(frame_index);
        OCSurfaceData surf = cur_surfaces().read(dispatch_id());
        $if(surf.hit->is_miss()) {
            $return();
        };
    };
    _shader1 = async([&, kernel = ocarina::move(kernel)] {
        return device().compile(kernel, "indirect spatial reuse and shading");
    });
}

CommandList ReSTIRIndirectIllumination::estimate(uint frame_index) const noexcept {
    CommandList ret;
    const Pipeline *rp = pipeline();
    if (_open) {
        ret << _shader0.get()(frame_index).dispatch(rp->resolution());
        ret << _shader1.get()(frame_index).dispatch(rp->resolution());
    }
    return ret;
}

void ReSTIRIndirectIllumination::prepare() noexcept {
    using ReSTIRIndirect::Reservoir;
    Pipeline *rp = pipeline();

    _reservoirs.super() = device().create_buffer<Reservoir>(rp->pixel_num() * 2,
                                                            "ReSTIRIndirectIllumination::_reservoirs x 2");
    _reservoirs.register_self(0, rp->pixel_num());
    _reservoirs.register_view(rp->pixel_num(), rp->pixel_num());
    vector<Reservoir> host{rp->pixel_num() * 2, Reservoir{}};
    _reservoirs.upload_immediately(host.data());

    using ReSTIRIndirect::RSVSample;
    _init_samples.super() = device().create_buffer<RSVSample>(rp->pixel_num(),
                                                              "ReSTIRIndirectIllumination::_init_samples");
    _init_samples.register_self();
    vector<RSVSample> vec{rp->pixel_num(), RSVSample{}};
    _init_samples.upload_immediately(vec.data());
}

}// namespace vision