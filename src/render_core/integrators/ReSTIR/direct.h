//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "util.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"

namespace vision {
/**
 * generate initial candidates
 * evaluate visibility for initial candidates
 * temporal reuse
 * spatial reuse and iterate
 */
class ReSTIRDirectIllumination : public SerialObject, public Ctx {
private:
    uint M{};
    bool _mis{};

    SpatialResamplingParam _spatial;
    TemporalResamplingParam _temporal;

    mutable RegistrableManaged<Reservoir> _reservoirs;
    mutable RegistrableManaged<Reservoir> _prev_reservoirs;
    RegistrableManaged<SurfaceData> &_surfaces;
    RegistrableManaged<SurfaceData> &_prev_surfaces;
    RegistrableManaged<float2> &_motion_vectors;

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
    ReSTIRDirectIllumination(const ParameterSet &desc, RegistrableManaged<float2> &motion_vec,
                             RegistrableManaged<SurfaceData> &surfaces,
                             RegistrableManaged<SurfaceData> &prev_surfaces);

    void prepare() noexcept;
    void compile() noexcept {
        compile_shader0();
        compile_shader1();
    }
    [[nodiscard]] DIReservoir RIS(Bool hit, const Interaction &it, SampledWavelengths &swl,
                                  const Uint &frame_index) const noexcept;
    [[nodiscard]] static Float compute_p_hat(const Interaction &it,
                                             SampledWavelengths &swl,
                                             const DIRSVSample &sample,
                                             LightSample *output_ls = nullptr) noexcept;
    [[nodiscard]] DIReservoir combine_reservoirs_MIS(DIReservoir cur_rsv,
                                                     SampledWavelengths &swl,
                                                     const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] DIReservoir combine_reservoirs(DIReservoir cur_rsv,
                                                 SampledWavelengths &swl,
                                                 const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] DIReservoir combine_reservoir(const DIReservoir &r0,
                                                OCSurfaceData cur_surf,
                                                const DIReservoir &r1,
                                                SampledWavelengths &swl) const noexcept;
    [[nodiscard]] DIReservoir combine_reservoir_MIS(DIReservoir r0,
                                                    OCSurfaceData s0,
                                                    DIReservoir r1,
                                                    OCSurfaceData S1,
                                                    SampledWavelengths &swl) const noexcept;
    [[nodiscard]] Float2 compute_motion_vec(const Float2 &p_film, const Float3 &cur_pos,
                                            const Bool &is_hit) const noexcept;
    [[nodiscard]] DIReservoir spatial_reuse(DIReservoir rsv,
                                            const OCSurfaceData &cur_surf,
                                            const Int2 &pixel,
                                            SampledWavelengths &swl,
                                            const Uint &frame_index) const noexcept;
    [[nodiscard]] DIReservoir temporal_reuse(DIReservoir rsv,
                                             const OCSurfaceData& cur_surf,
                                             const Float2 &motion_vec,
                                             const SensorSample &ss,
                                             SampledWavelengths &swl,
                                             const Uint &frame_index) const noexcept;
    [[nodiscard]] Float3 shading(const DIReservoir &rsv, const OCHit &hit,
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