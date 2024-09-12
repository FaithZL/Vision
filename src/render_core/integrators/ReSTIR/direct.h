//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "base.h"

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
class ReSTIRDI : public ReSTIR {
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
    SP<ScreenBuffer> radiance_{make_shared<ScreenBuffer>("ReSTIRDI::radiance_")};
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
    [[nodiscard]] static TSampler &sampler() noexcept { return scene().sampler(); }

public:
    ReSTIRDI() = default;
    ReSTIRDI(IlluminationIntegrator *integrator, const ParameterSet &desc);
    VS_HOTFIX_MAKE_RESTORE(RuntimeObject, integrator_, M_light_, M_bsdf_,
                           max_age_, debias_, pairwise_, reweight_, open_,
                           spatial_, temporal_, radiance_, reservoirs_, shader0_, shader1_)
    OC_MAKE_MEMBER_GETTER(open, )
    OC_MAKE_MEMBER_GETTER(radiance, &)
    [[nodiscard]] float factor() const noexcept { return static_cast<float>(open()); }
    void prepare() noexcept;
    void compile() noexcept {
        compile_shader0();
        compile_shader1();
    }
    OC_MAKE_MEMBER_SETTER(integrator)
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
                                   param.t_depth) &&
               cond;
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
    [[nodiscard]] HOTFIX_VIRTUAL DIReservoir RIS(const Bool &hit, const Interaction &it, const Var<direct::Param> &param,
                                                 Uint *flag) const noexcept;

    [[nodiscard]] HOTFIX_VIRTUAL SurfaceDataVar compute_hit(RayState rs, HitVar &hit, Interaction &it,
                                                            SurfaceExtendVar &surf_ext) const noexcept;

    /// evaluate Li from light
    [[nodiscard]] HOTFIX_VIRTUAL SampledSpectrum Li(const Interaction &it, MaterialEvaluator *bsdf,
                                                    const DIRSVSample &sample, LightSample *output_ls = nullptr,
                                                    Float *bsdf_pdf_point = nullptr) const noexcept;
    /// evaluate Li from bsdf
    [[nodiscard]] HOTFIX_VIRTUAL SampledSpectrum Li(const Interaction &it, MaterialEvaluator *bsdf,
                                                    DIRSVSample *sample, BSDFSample *bs, Float *light_pdf_point,
                                                    HitBSDFVar *hit_bsdf, Uint *flag) const noexcept;

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
    HOTFIX_VIRTUAL DIReservoir pairwise_combine(const DIReservoir &canonical_rsv, const Container<uint> &rsv_idx) const noexcept;

    /**
     * @return The weight of the return value is added to the canonical sample
     */
    [[nodiscard]] HOTFIX_VIRTUAL Float neighbor_pairwise_MIS(const DIReservoir &canonical_rsv, const Interaction &canonical_it,
                                                             const DIReservoir &other_rsv, const Interaction &other_it, Uint M,
                                                             DIReservoir *output_rsv) const noexcept;
    HOTFIX_VIRTUAL void canonical_pairwise_MIS(const DIReservoir &canonical_rsv, Float canonical_weight,
                                               DIReservoir *output_rsv) const noexcept;

    [[nodiscard]] HOTFIX_VIRTUAL DIReservoir constant_combine(const DIReservoir &canonical_rsv,
                                                              const Container<uint> &rsv_idx) const noexcept;

    [[nodiscard]] HOTFIX_VIRTUAL DIReservoir combine_spatial(DIReservoir cur_rsv,
                                                             const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL DIReservoir combine_temporal(const DIReservoir &cur_rsv,
                                                              const SurfaceDataVar &cur_surf,
                                                              DIReservoir &other_rsv,
                                                              Float3 view_pos,
                                                              Float3 prev_view_pos) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL DIReservoir spatial_reuse(DIReservoir rsv,
                                                           const SurfaceDataVar &cur_surf,
                                                           const Int2 &pixel,
                                                           const Var<Param> &param) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL DIReservoir temporal_reuse(DIReservoir rsv,
                                                            const SurfaceDataVar &cur_surf,
                                                            const Float2 &motion_vec,
                                                            const SensorSample &ss,
                                                            const Var<Param> &param) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL Float3 shading(DIReservoir rsv, const SurfaceDataVar &surf) const noexcept;
    HOTFIX_VIRTUAL void compile_shader0() noexcept;
    HOTFIX_VIRTUAL void compile_shader1() noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL direct::Param construct_param() const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL CommandList dispatch(uint frame_index) const noexcept;
};

}// namespace vision