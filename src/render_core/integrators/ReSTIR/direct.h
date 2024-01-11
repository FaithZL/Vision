//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "util.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"
#include "core/thread_pool.h"

namespace vision {

class IlluminationIntegrator;

/**
 * generate initial candidates
 * evaluate visibility for initial candidates
 * temporal reuse
 * spatial reuse and iterate
 */
class ReSTIRDirectIllumination : public SerialObject, public Ctx {
private:
    const IlluminationIntegrator *_integrator{};
    uint M_light{};
    uint M_bsdf{};
    bool _debias{false};
    bool _pairwise{false};
    bool _reweight{false};

    SpatialResamplingParam _spatial;
    TemporalResamplingParam _temporal;

    mutable RegistrableBuffer<Reservoir> _reservoirs;

    RegistrableBuffer<SurfaceData> &_surfaces0;
    RegistrableBuffer<SurfaceData> &_surfaces1;
    RegistrableBuffer<float2> &_motion_vectors;

    optional<Uint> _frame_index;

    /**
     * generate initial candidates
     * check visibility
     */
    std::shared_future<Shader<void(uint)>> _shader0;
    /**
     * spatial temporal reuse and shading
     */
    std::shared_future<Shader<void(uint)>> _shader1;

protected:
    [[nodiscard]] static Sampler *sampler() noexcept { return scene().sampler(); }

public:
    ReSTIRDirectIllumination(IlluminationIntegrator *integrator, const ParameterSet &desc,
                             RegistrableBuffer<float2> &motion_vec,
                             RegistrableBuffer<SurfaceData> &surfaces0,
                             RegistrableBuffer<SurfaceData> &surfaces1);

    void prepare() noexcept;
    void compile() noexcept {
        compile_shader0();
        compile_shader1();
    }
    [[nodiscard]] Bool is_neighbor(const OCSurfaceData &cur_surface,
                                   const OCSurfaceData &another_surface) const noexcept {
        return vision::is_neighbor(cur_surface, another_surface,
                                   _spatial.dot_threshold,
                                   _spatial.depth_threshold);
    }
    [[nodiscard]] Bool is_temporal_valid(const OCSurfaceData &cur_surface,
                                         const OCSurfaceData &prev_surface) const noexcept {
        return vision::is_neighbor(cur_surface, prev_surface,
                                   _temporal.dot_threshold,
                                   _temporal.depth_threshold);
    }
    [[nodiscard]] uint reservoir_base() const noexcept { return _reservoirs.index().hv(); }
    [[nodiscard]] uint surface_base() const noexcept { return _surfaces0.index().hv(); }
    [[nodiscard]] BindlessArrayBuffer<Reservoir> prev_reservoirs() const noexcept {
        return pipeline()->buffer<Reservoir>((_frame_index.value() & 1) + reservoir_base());
    }
    [[nodiscard]] BindlessArrayBuffer<Reservoir> passthrough_reservoirs() const noexcept {
        return pipeline()->buffer<Reservoir>(2 + reservoir_base());
    }
    [[nodiscard]] BindlessArrayBuffer<Reservoir> cur_reservoirs() const noexcept {
        return pipeline()->buffer<Reservoir>(((_frame_index.value() + 1) & 1) + reservoir_base());
    }
    [[nodiscard]] BindlessArrayBuffer<SurfaceData> prev_surfaces() const noexcept {
        return pipeline()->buffer<SurfaceData>((_frame_index.value() & 1) + surface_base());
    }
    [[nodiscard]] BindlessArrayBuffer<SurfaceData> cur_surfaces() const noexcept {
        return pipeline()->buffer<SurfaceData>(((_frame_index.value() + 1) & 1) + surface_base());
    }
    [[nodiscard]] DIReservoir RIS(Bool hit, const Interaction &it, SampledWavelengths &swl,
                                  const Uint &frame_index) const noexcept;

    /// evaluate Li from light
    [[nodiscard]] static SampledSpectrum Li(const Interaction &it, MaterialEvaluator *bsdf, const SampledWavelengths &swl,
                                            const DIRSVSample &sample, LightSample *output_ls = nullptr, Float *bsdf_pdf_point = nullptr) noexcept;
    /// evaluate Li from bsdf
    [[nodiscard]] static SampledSpectrum Li(const Interaction &it, MaterialEvaluator *bsdf, const SampledWavelengths &swl,
                                            DIRSVSample *sample, BSDFSample *bs, Float *light_pdf_point = nullptr) noexcept;

    template<typename... Args>
    [[nodiscard]] static Float compute_p_hat(Args &&...args) noexcept {
        SampledSpectrum f = Li(OC_FORWARD(args)...);
        Float p_hat = luminance(f.vec3());
        return p_hat;
    }

    /**
     * reference from https://intro-to-restir.cwyman.org/presentations/2023ReSTIR_Course_Notes.pdf equation 7.3
     *
     *  1 is canonical technique
     *
     *                    pi(x)
     * mi(x) = ---------------------------    i != 1    neighbor technique
     *           p1(x) + (M - 1) * pi(x)
     *
     *            1         M              p1(x)
     * m1(x) = -------  * sigma ----------------------------------    canonical technique
     *          M - 1      i=2     p1(x) + (M - 1) * pi(x)
     *
     */
    DIReservoir pairwise_combine(const DIReservoir &canonical_rsv,const Container<uint> &rsv_idx,
                                 const SampledWavelengths &swl) const noexcept;

    /**
     * @return The weight of the return value is added to the canonical sample
     */
    [[nodiscard]] Float neighbor_pairwise_MIS(const DIReservoir &canonical_rsv, const Interaction &canonical_it,
                                              const DIReservoir &other_rsv, const Interaction &other_it, Uint M,
                                              const SampledWavelengths &swl,
                                              DIReservoir *output_rsv) const noexcept;
    void canonical_pairwise_MIS(const DIReservoir &canonical_rsv, Float canonical_weight, const SampledWavelengths &swl,
                                DIReservoir *output_rsv) const noexcept;

    [[nodiscard]] DIReservoir constant_combine(const DIReservoir &canonical_rsv,const Container<uint> &rsv_idx,
                                               const SampledWavelengths &swl) const noexcept;

    [[nodiscard]] DIReservoir combine_spatial(DIReservoir cur_rsv,
                                              SampledWavelengths &swl,
                                              const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] DIReservoir combine_temporal(const DIReservoir &cur_rsv,
                                               OCSurfaceData cur_surf,
                                               const DIReservoir &other_rsv,
                                               SampledWavelengths &swl) const noexcept;
    [[nodiscard]] Float2 compute_motion_vec(const Float2 &p_film, const Float3 &cur_pos,
                                            const Bool &is_hit) const noexcept;
    [[nodiscard]] DIReservoir spatial_reuse(DIReservoir rsv,
                                            const OCSurfaceData &cur_surf,
                                            const Int2 &pixel,
                                            SampledWavelengths &swl,
                                            const Uint &frame_index) const noexcept;
    [[nodiscard]] DIReservoir temporal_reuse(DIReservoir rsv,
                                             const OCSurfaceData &cur_surf,
                                             const Float2 &motion_vec,
                                             const SensorSample &ss,
                                             SampledWavelengths &swl,
                                             const Uint &frame_index) const noexcept;
    [[nodiscard]] Float3 shading(DIReservoir rsv, const OCHit &hit,
                                 SampledWavelengths &swl, const Uint &frame_index) const noexcept;
    void compile_shader0() noexcept;
    void compile_shader1() noexcept;
    [[nodiscard]] CommandList estimate(uint frame_index) const noexcept;
};

}// namespace vision