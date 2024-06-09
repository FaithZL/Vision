//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "util.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"
#include "core/thread_pool.h"

namespace vision::direct {
struct Param {
    uint M_light{};
    uint M_bsdf{};
    uint max_age{};

    /// spatial
    uint spatial{1};
    uint N{};
    float s_dot{};
    float s_depth{};
    float s_radius{};

    /// temporal
    uint temporal{1};
    uint history_limit{};
    float t_dot{};
    float t_depth{};
    float t_radius{};
};
}// namespace vision::direct

OC_PARAM_STRUCT(vision::direct, Param, M_light, M_bsdf, max_age, spatial, N,
                s_dot, s_depth, s_radius, temporal, history_limit,
                t_dot, t_depth, t_radius){};

namespace vision {

class RayTracingIntegrator;

/**
 * generate initial candidates
 * evaluate visibility for initial candidates
 * temporal reuse
 * spatial reuse and iterate
 */
class ReSTIRDI : public SerialObject, public Context, public RenderEnv, public GUI {
private:
    IlluminationIntegrator *integrator_{};
    uint M_light_{};
    uint M_bsdf_{};
    uint max_age_{};
    bool debias_{false};
    bool pairwise_{false};
    bool reweight_{false};
    bool open_{true};

    SpatialResamplingParam spatial_;
    TemporalResamplingParam temporal_;
    ScreenBuffer radiance_{"ReSTIRDI::radiance_"};
    mutable RegistrableBuffer<Reservoir> reservoirs_{pipeline()->bindless_array()};

    /**
     * generate initial candidates
     * check visibility
     * temporal reuse
     */
    Shader<void(uint, direct::Param)> shader0_;
    /**
     * spatial reuse and shading
     */
    Shader<void(uint, direct::Param)> shader1_;

protected:
    [[nodiscard]] static Sampler &sampler() noexcept { return scene().sampler(); }

public:
    ReSTIRDI(IlluminationIntegrator *integrator, const ParameterSet &desc);
    OC_MAKE_MEMBER_GETTER(open, )
    OC_MAKE_MEMBER_GETTER(radiance, &)
    [[nodiscard]] float factor() const noexcept { return static_cast<float>(open()); }
    void prepare() noexcept;
    void compile() noexcept {
        compile_shader0();
        compile_shader1();
    }

    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] static Bool is_neighbor(const SurfaceDataVar &cur_surface,
                                          const SurfaceDataVar &another_surface,
                                          const Var<direct::Param> &param) noexcept {
        return vision::is_neighbor(cur_surface, another_surface,
                                   param.s_dot,
                                   param.s_depth);
    }
    [[nodiscard]] static Bool is_temporal_valid(const SurfaceDataVar &cur_surface,
                                                const SurfaceDataVar &prev_surface,
                                                const Var<direct::Param> &param,
                                                DIRSVSample *sample) noexcept {
        Bool cond = sample ? sample->age < param.max_age : true;
        return vision::is_neighbor(cur_surface, prev_surface,
                                   param.t_dot,
                                   param.t_depth) && cond;
    }
    [[nodiscard]] uint reservoir_base() const noexcept { return reservoirs_.index().hv(); }
    [[nodiscard]] auto prev_reservoirs() const noexcept {
        return pipeline()->buffer_var<Reservoir>((frame_index() & 1) + reservoir_base());
    }
    [[nodiscard]] auto passthrough_reservoirs() const noexcept {
        return pipeline()->buffer_var<Reservoir>(2 + reservoir_base());
    }
    [[nodiscard]] auto cur_reservoirs() const noexcept {
        return pipeline()->buffer_var<Reservoir>(((frame_index() + 1) & 1) + reservoir_base());
    }
    [[nodiscard]] auto prev_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().prev_surfaces_index(frame_index()));
    }
    [[nodiscard]] auto cur_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().cur_surfaces_index(frame_index()));
    }
    [[nodiscard]] DIReservoir RIS(Bool hit, const Interaction &it,
                                  const Var<direct::Param> &param) const noexcept;

    /// evaluate Li from light
    [[nodiscard]] SampledSpectrum Li(const Interaction &it, MaterialEvaluator *bsdf,
                                     const DIRSVSample &sample, LightSample *output_ls = nullptr,
                                     Float *bsdf_pdf_point = nullptr) const noexcept;
    /// evaluate Li from bsdf
    [[nodiscard]] SampledSpectrum Li(const Interaction &it, MaterialEvaluator *bsdf,
                                     DIRSVSample *sample, BSDFSample *bs, Float *light_pdf_point,
                                     HitBSDFVar *hit_bsdf) const noexcept;

    template<typename... Args>
    [[nodiscard]] Float compute_p_hat(Args &&...args) const noexcept {
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
    DIReservoir pairwise_combine(const DIReservoir &canonical_rsv, const Container<uint> &rsv_idx) const noexcept;

    /**
     * @return The weight of the return value is added to the canonical sample
     */
    [[nodiscard]] Float neighbor_pairwise_MIS(const DIReservoir &canonical_rsv, const Interaction &canonical_it,
                                              const DIReservoir &other_rsv, const Interaction &other_it, Uint M,
                                              DIReservoir *output_rsv) const noexcept;
    void canonical_pairwise_MIS(const DIReservoir &canonical_rsv, Float canonical_weight,
                                DIReservoir *output_rsv) const noexcept;

    [[nodiscard]] DIReservoir constant_combine(const DIReservoir &canonical_rsv,
                                               const Container<uint> &rsv_idx) const noexcept;

    [[nodiscard]] DIReservoir combine_spatial(DIReservoir cur_rsv,
                                              const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] DIReservoir combine_temporal(const DIReservoir &cur_rsv,
                                               SurfaceDataVar cur_surf,
                                               DIReservoir &other_rsv) const noexcept;
    [[nodiscard]] DIReservoir spatial_reuse(DIReservoir rsv,
                                            const SurfaceDataVar &cur_surf,
                                            const Int2 &pixel,
                                            const Var<Param> &param) const noexcept;
    [[nodiscard]] DIReservoir temporal_reuse(DIReservoir rsv,
                                             const SurfaceDataVar &cur_surf,
                                             const Float2 &motion_vec,
                                             const SensorSample &ss,
                                             const Var<Param> &param) const noexcept;
    [[nodiscard]] Float3 shading(DIReservoir rsv, const HitVar &hit) const noexcept;
    void compile_shader0() noexcept;
    void compile_shader1() noexcept;
    [[nodiscard]] direct::Param construct_param() const noexcept;
    [[nodiscard]] CommandList estimate(uint frame_index) const noexcept;
};

}// namespace vision