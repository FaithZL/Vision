//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "common.h"

namespace vision {
struct DIParam {
    uint M_light{};
    uint M_bsdf{};
    uint max_age{};
    float diff_factor{};

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
}// namespace vision

OC_PARAM_STRUCT(vision, DIParam, M_light, M_bsdf, max_age, diff_factor, spatial, N,
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
    uint M_light_{};
    uint M_bsdf_{};
    bool debias_{false};
    bool pairwise_{false};
    bool reweight_{false};
    uint max_recursion_{};
    SP<ScreenBuffer> radiance_{make_shared<ScreenBuffer>("ReSTIRDI::radiance_")};
    mutable RegistrableBuffer<DIReservoir> reservoirs_{pipeline()->bindless_array()};

    /**
     * generate initial candidates
     * check visibility
     * temporal reuse
     */
    Shader<void(uint, DIParam)> shader0_;
    /**
     * spatial reuse and shading
     */
    Shader<void(uint, DIParam)> shader1_;

protected:
    [[nodiscard]] static TSampler &sampler() noexcept { return scene().sampler(); }

public:
    ReSTIRDI() = default;
    ReSTIRDI(IlluminationIntegrator *integrator, const ParameterSet &desc);
    VS_HOTFIX_MAKE_RESTORE(ReSTIR, M_light_, M_bsdf_, debias_, pairwise_, reweight_, max_recursion_,
                           radiance_, reservoirs_, shader0_, shader1_)
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
    [[nodiscard]] static Bool is_valid_neighbor(const SurfaceDataVar &cur_surface,
                                                const SurfaceDataVar &another_surface,
                                                const Var<DIParam> &param) noexcept {
        return vision::is_valid_neighbor(cur_surface, another_surface,
                                         param.s_dot, param.s_depth, param.diff_factor);
    }
    void update_resolution(uint2 res) noexcept override;
    [[nodiscard]] static Bool is_temporal_valid(const SurfaceDataVar &cur_surface,
                                                const SurfaceDataVar &prev_surface,
                                                const Var<DIParam> &param,
                                                DISampleVar *sample) noexcept {
        Bool cond = sample ? sample->age < param.max_age : true;
        return vision::is_valid_neighbor(cur_surface, prev_surface,
                                         param.t_dot,
                                         param.t_depth, param.diff_factor) &&
               cond;
    }
    [[nodiscard]] uint reservoir_base() const noexcept { return reservoirs_.index().hv(); }
    [[nodiscard]] auto prev_reservoirs() const noexcept {
        return pipeline()->buffer_var<DIReservoir>((frame_index() & 1) + reservoir_base());
    }
    [[nodiscard]] auto passthrough_reservoirs() const noexcept {
        return pipeline()->buffer_var<DIReservoir>(2 + reservoir_base());
    }
    [[nodiscard]] auto cur_reservoirs() const noexcept {
        return pipeline()->buffer_var<DIReservoir>(((frame_index() + 1) & 1) + reservoir_base());
    }
    [[nodiscard]] HOTFIX_VIRTUAL DIReservoirVar RIS(const Bool &hit, const Interaction &it, const Var<DIParam> &param,
                                                    const Float3 &throughput, Uint *flag) const noexcept;

    [[nodiscard]] HOTFIX_VIRTUAL SurfaceDataVar compute_hit(RayState rs, TriangleHitVar &hit, Interaction &it,
                                                            SurfaceExtendVar &surf_ext) const noexcept;

    /// evaluate Li from light
    [[nodiscard]] HOTFIX_VIRTUAL SampledSpectrum Li(const Interaction &it, MaterialEvaluator *bsdf,
                                                    const DISampleVar &sample, LightSample *output_ls = nullptr,
                                                    Float *bsdf_pdf_point = nullptr) const noexcept;
    /// evaluate Li from bsdf
    [[nodiscard]] HOTFIX_VIRTUAL SampledSpectrum Li(const Interaction &it, MaterialEvaluator *bsdf,
                                                    DISampleVar *sample, BSDFSample *bs, Float *light_pdf_point,
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
    HOTFIX_VIRTUAL DIReservoirVar pairwise_combine(const DIReservoirVar &canonical_rsv, Float3 view_pos,
                                                   const Container<uint> &rsv_idx) const noexcept;

    /**
     * @return The weight of the return value is added to the canonical sample
     */
    [[nodiscard]] HOTFIX_VIRTUAL Float neighbor_pairwise_MIS(const DIReservoirVar &canonical_rsv, const Interaction &canonical_it,
                                                             const DIReservoirVar &other_rsv, const Interaction &other_it, Uint M,
                                                             DIReservoirVar *output_rsv) const noexcept;
    HOTFIX_VIRTUAL void canonical_pairwise_MIS(const DIReservoirVar &canonical_rsv, Float canonical_weight,
                                               DIReservoirVar *output_rsv) const noexcept;

    [[nodiscard]] HOTFIX_VIRTUAL DIReservoirVar constant_combine(const DIReservoirVar &canonical_rsv, Float3 view_pos,
                                                                 const Container<uint> &rsv_idx) const noexcept;

    [[nodiscard]] HOTFIX_VIRTUAL DIReservoirVar combine_spatial(DIReservoirVar cur_rsv, Float3 view_pos,
                                                                const Container<uint> &rsv_idx) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL DIReservoirVar combine_temporal(const DIReservoirVar &cur_rsv,
                                                                 const SurfaceDataVar &cur_surf,
                                                                 DIReservoirVar &other_rsv,
                                                                 Float3 view_pos,
                                                                 Float3 prev_view_pos) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL DIReservoirVar spatial_reuse(DIReservoirVar rsv,
                                                              const SurfaceDataVar &cur_surf,
                                                              const Int2 &pixel,
                                                              const Var<DIParam> &param) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL DIReservoirVar temporal_reuse(DIReservoirVar rsv,
                                                               const SurfaceDataVar &cur_surf,
                                                               const Float2 &motion_vec,
                                                               const SensorSample &ss,
                                                               const Var<DIParam> &param) const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL Float3 shading(DIReservoirVar rsv, const SurfaceDataVar &surf) const noexcept;
    HOTFIX_VIRTUAL void compile_shader0() noexcept;
    HOTFIX_VIRTUAL void compile_shader1() noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL DIParam construct_param() const noexcept;
    [[nodiscard]] HOTFIX_VIRTUAL CommandList dispatch(uint frame_index) const noexcept;
};

}// namespace vision