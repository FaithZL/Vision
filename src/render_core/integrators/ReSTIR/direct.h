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
    mutable RegistrableManaged<SurfaceData> _surfaces;
    mutable RegistrableManaged<SurfaceData> _prev_surfaces;
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
    ReSTIRDirectIllumination(const ParameterSet &desc, RegistrableManaged<float2> &motion_vec);

    void prepare() noexcept;
    void compile() noexcept {
        compile_shader0();
        compile_shader1();
    }
    [[nodiscard]] OCReservoir RIS(Bool hit, const Interaction &it, SampledWavelengths &swl,
                                  const Uint &frame_index) const noexcept;
    [[nodiscard]] static Float compute_p_hat(const Interaction &it,
                                             SampledWavelengths &swl,
                                             const OCRSVSample &sample,
                                             LightSample *output_ls = nullptr) noexcept;
    [[nodiscard]] OCReservoir combine_reservoirs_MIS(OCReservoir cur_rsv,
                                                     SampledWavelengths &swl,
                                                     const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] OCReservoir combine_reservoirs(OCReservoir cur_rsv,
                                                 SampledWavelengths &swl,
                                                 const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] OCReservoir combine_reservoir(const OCReservoir &r0,
                                                const OCReservoir &r1,
                                                SampledWavelengths &swl) const noexcept;
    [[nodiscard]] Float2 compute_motion_vec(const Float2 &p_film, const Float3 &cur_pos,
                                            const Bool &is_hit) const noexcept;
    [[nodiscard]] OCReservoir spatial_reuse(OCReservoir rsv,
                                            const OCSurfaceData &cur_surf,
                                            const Int2 &pixel,
                                            SampledWavelengths &swl,
                                            const Uint &frame_index) const noexcept;
    [[nodiscard]] OCReservoir temporal_reuse(OCReservoir rsv,
                                             const OCSurfaceData& cur_surf,
                                             const SensorSample &ss,
                                             SampledWavelengths &swl,
                                             const Uint &frame_index) const noexcept;
    [[nodiscard]] Float3 shading(const OCReservoir &rsv, const OCHit &hit,
                                 SampledWavelengths &swl, const Uint &frame_index) const noexcept;
    [[nodiscard]] Bool is_neighbor(const OCSurfaceData &cur_surface,
                                   const OCSurfaceData &another_surface) const noexcept;
    [[nodiscard]] Bool is_temporal_valid(const OCSurfaceData &cur_surface,
                                         const OCSurfaceData &prev_surface) const noexcept;
    void compile_shader0() noexcept;
    void compile_shader1() noexcept;
    [[nodiscard]] CommandList estimate() const noexcept;
};

}// namespace vision