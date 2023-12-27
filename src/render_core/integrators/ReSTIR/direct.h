//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "util.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"

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
    CorrectMode _correct_mode;
    bool _debias{false};

    SpatialResamplingParam _spatial;
    TemporalResamplingParam _temporal;

    mutable RegistrableManaged<Reservoir> _reservoirs0;
    mutable RegistrableManaged<Reservoir> _reservoirs1;
    mutable RegistrableManaged<Reservoir> _reservoirs2;
    RegistrableManaged<SurfaceData> &_surfaces0;
    RegistrableManaged<SurfaceData> &_surfaces1;
    RegistrableManaged<float2> &_motion_vectors;

    optional<Uint> _frame_index;

    /**
     * generate initial candidates
     * check visibility
     */
    Shader<void(uint)> _shader0;
    /**
     * spatial temporal reuse and shading
     */
    Shader<void(uint)> _shader1;

public:
    ReSTIRDirectIllumination(IlluminationIntegrator *integrator, const ParameterSet &desc,
                             RegistrableManaged<float2> &motion_vec,
                             RegistrableManaged<SurfaceData> &surfaces0,
                             RegistrableManaged<SurfaceData> &surfaces1);

    void prepare() noexcept;
    void compile() noexcept {
        compile_shader0();
        compile_shader1();
    }

    [[nodiscard]] uint reservoir_base() const noexcept { return _reservoirs0.index().hv(); }
    [[nodiscard]] uint surface_base() const noexcept { return _surfaces0.index().hv(); }
    [[nodiscard]] ResourceArrayBuffer<Reservoir> prev_reservoir() const noexcept;
    [[nodiscard]] ResourceArrayBuffer<Reservoir> passthrough_reservoir() const noexcept;
    [[nodiscard]] ResourceArrayBuffer<Reservoir> cur_reservoir() const noexcept;
    [[nodiscard]] ResourceArrayBuffer<SurfaceData> prev_surface() const noexcept;
    [[nodiscard]] ResourceArrayBuffer<SurfaceData> cur_surface() const noexcept;
    [[nodiscard]] DIReservoir RIS(Bool hit, const Interaction &it, SampledWavelengths &swl,
                                  const Uint &frame_index) const noexcept;
    /// sample Li from light
    [[nodiscard]] static SampledSpectrum sample_Li(const Interaction &it, MaterialEvaluator *bsdf, const SampledWavelengths &swl,
                                                   const DIRSVSample &sample, LightSample *output_ls = nullptr) noexcept;
    template<typename... Args>
    [[nodiscard]] static Float compute_p_hat(Args &&...args) noexcept {
        SampledSpectrum f = sample_Li(OC_FORWARD(args)...);
        Float p_hat = luminance(f.vec3());
        return p_hat;
    }
    [[nodiscard]] DIReservoir combine_reservoirs(DIReservoir cur_rsv,
                                                 SampledWavelengths &swl,
                                                 const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] DIReservoir combine_reservoir(const DIReservoir &cur_rsv,
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
    [[nodiscard]] Bool is_neighbor(const OCSurfaceData &cur_surface,
                                   const OCSurfaceData &another_surface) const noexcept;
    [[nodiscard]] Bool is_temporal_valid(const OCSurfaceData &cur_surface,
                                         const OCSurfaceData &prev_surface) const noexcept;
    void compile_shader0() noexcept;
    void compile_shader1() noexcept;
    [[nodiscard]] CommandList estimate(uint frame_index) const noexcept;
};

}// namespace vision