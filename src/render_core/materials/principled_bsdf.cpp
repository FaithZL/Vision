//
// Created by Zero on 2025/1/16.
//

#include <utility>
#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"
#include "ltc_sheen_table.inl.h"
#include "base/scattering/precomputed_table.inl.h"

namespace vision {

class FresnelGeneralizedSchlick : public Fresnel {
private:
    SampledSpectrum F0_;
    SampledSpectrum eta_;

public:
    FresnelGeneralizedSchlick(SampledSpectrum F0, SampledSpectrum eta,
                              const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)),
          eta_(std::move(eta)) {}
    FresnelGeneralizedSchlick(SampledSpectrum F0, const Float& eta,
                              const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)),
          eta_(SampledSpectrum{1, eta}) {}
    OC_MAKE_MEMBER_GETTER(F0, )
    void set_eta(const vision::SampledSpectrum &eta) noexcept override {
        eta_ = eta;
    }
    [[nodiscard]] SampledSpectrum evaluate(ocarina::Float cos_theta) const noexcept override {
        Float F_real = fresnel_dielectric(cos_theta, eta_[0]);
        Float F0_real = schlick_F0_from_ior(eta_[0]);
        Float t = inverse_lerp(F_real, F0_real, 1.f);
        t = ocarina::clamp(t, 0.f, 1.f);
        SampledSpectrum ret = lerp(t, F0_, 1.f);
        return ret;
    }
    [[nodiscard]] SampledSpectrum eta() const noexcept override {
        return eta_;
    }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelGeneralizedSchlick)
};

class FresnelF82Tint : public Fresnel {
private:
    SampledSpectrum F0_;
    SampledSpectrum B_;

public:
    using Fresnel::Fresnel;

    FresnelF82Tint(SampledSpectrum F0, SampledSpectrum B,
                   const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)), B_(std::move(B)) {
    }

    FresnelF82Tint(SampledSpectrum F0, const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)), B_(SampledSpectrum::one(swl.dimension())) {}

    void init_from_F82(const SampledSpectrum& F82) {
        static constexpr float f = 6.f / 7.f;
        static constexpr float f5 = Pow<5>(f);
        SampledSpectrum one = SampledSpectrum::one(swl_->dimension());
        SampledSpectrum f_schlick = lerp(f5, F0_, one);
        B_ = f_schlick * (7.f / (f5 * f)) * (one - F82);
    }

    [[nodiscard]] SampledSpectrum evaluate(ocarina::Float cos_theta) const noexcept override {
        Float mu = ocarina::saturate(1.f - cos_theta);
        Float mu5 = Pow<5>(mu);
        SampledSpectrum f_schlick = lerp(mu5, F0_, SampledSpectrum::one(swl_->dimension()));
        SampledSpectrum ret = saturate(f_schlick - B_ * cos_theta * mu5 * mu);
        return ret;
    }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelF82Tint)
};

/// reference https://tizianzeltner.com/projects/Zeltner2022Practical/
/// reference https://github.com/tizian/ltc-sheen
class SheenLTC : public BxDFSet {
protected:
    const SampledWavelengths *swl_{nullptr};
    SampledSpectrum tint_;
    Float alpha_;
    Float a_;
    Float b_;

public:
    enum Mode : int {
        Volumetric,
        Approximate,
    };
    static constexpr const char *Volume = "SheenLTC::Volume";
    static constexpr const char *Approx = "SheenLTC::Approximate";
    static constexpr auto lut_res = 32;

public:
    SheenLTC(Mode mode, const Float &cos_theta, SampledSpectrum tint,
             Float alpha, const SampledWavelengths &swl)
        : tint_(std::move(tint)), alpha_(std::move(alpha)), swl_(&swl) {
        Float4 c = fetch_ltc(mode, cos_theta);
        a_ = c.x;
        b_ = c.y;
        tint_ *= c.z;
    }
    static void prepare() {
        MaterialLut::instance().load_lut(Volume, make_uint2(lut_res), PixelStorage::FLOAT4,
                                         addressof(SheenLTCTableVolume));
        MaterialLut::instance().load_lut(Approx, make_uint2(lut_res), PixelStorage::FLOAT4,
                                         addressof(SheenLTCTableApprox));
    }
    [[nodiscard]] Float4 fetch_ltc(Mode mode, const Float &cos_theta) {
        const char *name = mode == Volumetric ? Volume : Approx;
        return MaterialLut::instance().sample(name, 4, make_float2(cos_theta, alpha_)).as_vec4();
    }
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override {
        return tint_;
    }
    [[nodiscard]] Uint flag() const noexcept override { return BxDFFlag::GlossyRefl; }

    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode, const Uint &flag,
                                             TransportMode tm) const noexcept override {
        ScatterEval ret{*swl_};
        Float cos_theta_o = cos_theta(wo);
        Float cos_theta_i = cos_theta(wi);

        Float ltc_value = eval_ltc(wi);
        ret.f = ltc_value * tint_ / cos_theta_i;
        ret.pdfs = ltc_value;
        ret.f = select(cos_theta_i < 0 || cos_theta_o < 0, 0.f, ret.f);
        return ret;
    }
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                                          TransportMode tm) const noexcept override {
        BSDFSample ret{*swl()};
        SampledDirection sd = sample_wi(wo, flag, sampler);
        ret.eval = evaluate_local(wo, sd.wi, MaterialEvalMode::All, flag, tm);
        ret.wi = sd.wi;
        ret.eval.pdfs *= sd.factor();
        return ret;
    }

    /**
     *     [[1/a    0      -b/a   ]
     * M =  [0      1/a     0     ]
     *      [0      0       1     ]]
     * @param v
     * @return
     */
    [[nodiscard]] Float3 M(const Float3 &v) const noexcept {
        return make_float3(v.x / a_ - v.z * b_ / a_,
                           v.y / a_,
                           v.z);
    }

    /**
     *          [[a    0    b   ]
     * M^{-1} =  [0    a    0   ]
     *           [0    0    1   ]]
     * @param v
     * @return
     */
    [[nodiscard]] Float3 inv_M(const Float3 &v) const noexcept {
        return make_float3(a_ * v.x + b_ * v.z,
                           a_ * v.y,
                           v.z);
    }

    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override {

        Float3 wi = square_to_cosine_hemisphere(sampler->next_2d());
        wi = M(wi);
        SampledDirection sd;
        sd.wi = normalize(wi);
        return sd;
    }
    [[nodiscard]] Float eval_ltc(Float3 wi) const noexcept {
        wi = inv_M(wi);
        Float length = ocarina::length(wi);
        wi /= length;
        Float det = sqr(a_);
        Float jacobian = det / (length * length * length);
        return cosine_hemisphere_PDF(cos_theta(wi)) * jacobian;
    }
    [[nodiscard]] const SampledWavelengths *swl() const override { return swl_; }
};

class CoatBxDFSet : public MicrofacetBxDFSet {
public:
    using MicrofacetBxDFSet::MicrofacetBxDFSet;

    static constexpr float ior_lower = 1.003;
    static constexpr float ior_upper = 4.f;
    static constexpr const char *lut_name = "CoatBxDFSet::lut";
    static constexpr uint lut_res = 32;
    static void prepare() {
        MaterialLut::instance().load_lut(lut_name, make_uint3(lut_res),
                                         PixelStorage::FLOAT1,
                                         addressof(CoatBxDFSet_Table));
    }

    /// for precompute begin
    static constexpr const char *name = "CoatBxDFSet";
    static UP<CoatBxDFSet> create_for_precompute(const SampledWavelengths &swl) noexcept {
        SP<Fresnel> fresnel = make_shared<FresnelDielectric>(SampledSpectrum(swl, 1.5f), swl);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(make_float2(alpha_lower), true);
        UP<MicrofacetReflection> refl = make_unique<MicrofacetReflection>(SampledSpectrum::one(swl.dimension()),
                                                                          swl, microfacet);
        return make_unique<CoatBxDFSet>(fresnel, ocarina::move(refl));
    }
    void from_ratio_z(ocarina::Float z) noexcept override {
        Float ior = lerp(z, ior_lower, ior_upper);
        fresnel_->set_eta(SampledSpectrum(bxdf()->swl(), ior));
    }

    [[nodiscard]] Float to_ratio_z() const noexcept override {
        Float ior = fresnel_->eta().average();
        return inverse_lerp(ior, ior_lower, ior_upper);
    }
    /// for precompute end

    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override {
        Float x = to_ratio_x();
        Float z = to_ratio_z();
        Float3 uvw = make_float3(x, cos_theta, z);
        Float s = MaterialLut::instance().sample(lut_name, 1, uvw).as_scalar();
        SampledSpectrum ret = s * bxdf()->albedo(cos_theta);
        return ret;
    }
};

class MetallicBxDFSet : public MicrofacetBxDFSet {
public:
    using MicrofacetBxDFSet::MicrofacetBxDFSet;
};

class SpecularBxDFSet : public MicrofacetBxDFSet {
public:
    using MicrofacetBxDFSet::MicrofacetBxDFSet;

    static constexpr const char *lut_name = "SpecularBxDFSet::lut";
    static constexpr uint lut_res = 32;

    static void prepare() {
        MaterialLut::instance().load_lut(lut_name, make_uint3(lut_res),
                                         PixelStorage::FLOAT1,
                                         addressof(SpecularBxDFSet_Table));
    }

    /// for precompute begin
    static constexpr const char *name = "SpecularBxDFSet";
    static UP<SpecularBxDFSet> create_for_precompute(const SampledWavelengths &swl) noexcept {
        SampledSpectrum f0 = SampledSpectrum(make_float3(0.04));
        SampledSpectrum f90 = SampledSpectrum(make_float3(1));
        SP<Fresnel> fresnel_schlick = make_shared<FresnelGeneralizedSchlick>(f0, 1.5f, swl);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(0.00f, 0.0f, true);
        UP<MicrofacetBxDF> bxdf = make_unique<MicrofacetReflection>(SampledSpectrum::one(swl), swl, microfacet);
        return make_unique<SpecularBxDFSet>(fresnel_schlick, std::move(bxdf));
    }
    void from_ratio_z(ocarina::Float z) noexcept override {
        Float ior = schlick_ior_from_F0(Pow<4>(z));
        fresnel_->set_eta(SampledSpectrum(1, ior));
    }
    /// for precompute end

    [[nodiscard]] Float to_ratio_z() const noexcept override {
        Float ior = fresnel_->eta().average();
        return ior_to_ratio_z(ior);
    }

    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override {
        Float x = to_ratio_x();
        Float z = to_ratio_z();
        Float3 uvw = make_float3(x, cos_theta, z);
        Float s = MaterialLut::instance().sample(lut_name, 1, uvw).as_scalar();
        SampledSpectrum f0 = fresnel<FresnelGeneralizedSchlick>()->F0();
        SampledSpectrum ret = lerp(s, f0, SampledSpectrum::one(*swl())) * bxdf()->albedo(cos_theta);
        return ret;
    }
};

[[nodiscard]] inline SampledSpectrum layering_weight(const SampledSpectrum &layer_albedo,
                                                     const SampledSpectrum &weight) noexcept {
    SampledSpectrum tmp = safe_div(layer_albedo, weight);
    Float max_comp = tmp.max();
    return weight * saturate(1 - max_comp);
}

class PrincipledBSDF : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(metallic)
    VS_MAKE_SLOT(ior)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(spec_tint)
    VS_MAKE_SLOT(anisotropic)

    VS_MAKE_SLOT(sheen_weight)
    VS_MAKE_SLOT(sheen_roughness)
    VS_MAKE_SLOT(sheen_tint)

    VS_MAKE_SLOT(coat_weight)
    VS_MAKE_SLOT(coat_roughness)
    VS_MAKE_SLOT(coat_ior)
    VS_MAKE_SLOT(coat_tint)

    VS_MAKE_SLOT(subsurface_weight)
    VS_MAKE_SLOT(subsurface_radius)
    VS_MAKE_SLOT(subsurface_scale)

    VS_MAKE_SLOT(transmission_weight)
    VS_MAKE_SLOT(opcacity)
    SheenLTC::Mode sheen_mode_{SheenLTC::Approximate};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MultiBxDFSet)

public:
    PrincipledBSDF() = default;
    VS_MAKE_PLUGIN_NAME_FUNC
    explicit PrincipledBSDF(const MaterialDesc &desc)
        : Material(desc) {

#define INIT_SLOT(name, default_value, type) \
    name##_.set(Slot::create_slot(desc.slot(#name, default_value, type)))

        INIT_SLOT(color, make_float3(1.f), Albedo);
        INIT_SLOT(metallic, 0.f, Number);
        INIT_SLOT(ior, 1.5f, Number)->set_range(1.01, 20.f);
        INIT_SLOT(roughness, 0.5f, Number)->set_range(0.0001f, 1.f);
        INIT_SLOT(spec_tint, make_float3(0.f), Albedo);
        INIT_SLOT(anisotropic, 0.f, Number);

        INIT_SLOT(sheen_weight, 0.f, Number);
        INIT_SLOT(sheen_roughness, 0.5f, Number);
        INIT_SLOT(sheen_tint, make_float3(1.f), Albedo);

        INIT_SLOT(coat_weight, 0.f, Number);
        INIT_SLOT(coat_roughness, 0.2f, Number)->set_range(0.0001f, 1.f);
        INIT_SLOT(coat_ior, 1.5f, Number)->set_range(1.01, 4.f);
        INIT_SLOT(coat_tint, make_float3(1.f), Albedo);

        INIT_SLOT(subsurface_weight, 0.3f, Number);
        INIT_SLOT(subsurface_radius, make_float3(1.f), Number);
        INIT_SLOT(subsurface_scale, 0.2f, Number);

        INIT_SLOT(transmission_weight, 0.f, Number);
        INIT_SLOT(opcacity, 1.f, Number)->set_range(0.f, 1.f);

#undef INIT_SLOT
        init_slot_cursor(&color_, &opcacity_);
    }
    VS_HOTFIX_MAKE_RESTORE(Material, sheen_mode_)
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        static vector<const char *> names = {"volume", "approximate"};
        widgets->combo("sheen mode", reinterpret_cast<int *>(addressof(sheen_mode_)), names);
        Material::render_sub_UI(widgets);
    }
    void prepare() noexcept override {
        CoatBxDFSet::prepare();
        SheenLTC::prepare();
        SpecularBxDFSet::prepare();
        DielectricBxDFSet::prepare();
    }

    template<typename TLobe>
    [[nodiscard]] PrecomputedLobeTable precompute_lobe() const noexcept {
        return Material::precompute_lobe<TLobe>(make_uint3(TLobe::lut_res));
    }

    [[nodiscard]] vector<PrecomputedLobeTable> precompute() const noexcept override {
        vector<PrecomputedLobeTable> ret;
        ret.push_back(precompute_lobe<SpecularBxDFSet>());
        ret.push_back(precompute_lobe<CoatBxDFSet>());
        return ret;
    }
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        MultiBxDFSet::Lobes lobes;
        auto [color, color_lum] = color_.eval_albedo_spectrum(it, swl);
        DynamicArray<float> iors = ior_.evaluate(it, swl);
        Float ior = iors.as_scalar();
        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
        Float anisotropic = anisotropic_.evaluate(it, swl).as_scalar();
        SampledSpectrum specular_tint = spec_tint_.eval_albedo_spectrum(it, swl).sample;

        Float aspect = sqrt(1 - anisotropic * 0.9f);
        Float2 alpha = make_float2(max(0.001f, sqr(roughness) / aspect),
                                   max(0.001f, sqr(roughness) * aspect));
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);

        SampledSpectrum weight = SampledSpectrum::one(swl.dimension());
        Float cos_theta = dot(it.wo, it.shading.normal());
        Float front_factor = cast<float>(cos_theta > 0.f);
        {
            /// sheen
            SampledSpectrum sheen_tint = sheen_tint_.eval_albedo_spectrum(it, swl).sample;
            Float sheen_weight = sheen_weight_.evaluate(it, swl).as_scalar();
            Float sheen_roughness = sheen_roughness_.evaluate(it, swl).as_scalar();

            UP<SheenLTC> sheen_ltc = make_unique<SheenLTC>(sheen_mode_, cos_theta,
                                                           sheen_tint * sheen_weight * weight,
                                                           sheen_roughness, swl);
            SampledSpectrum sheen_albedo = sheen_ltc->albedo(cos_theta) * front_factor;
            WeightedBxDFSet sheen_lobe(sheen_albedo.average(), std::move(sheen_ltc));
            lobes.push_back(std::move(sheen_lobe));
            weight = layering_weight(sheen_albedo, weight);
        }
        {
            /// coat
            Float cc_weight = coat_weight_.evaluate(it, swl).as_scalar();
            Float cc_roughness = clamp(coat_roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
            cc_roughness = sqr(cc_roughness);
            Float cc_ior = coat_ior_.evaluate(it, swl).as_scalar();
            SampledSpectrum cc_tint = coat_tint_.eval_albedo_spectrum(it, swl).sample;
            SP<Fresnel> fresnel_cc = make_shared<FresnelDielectric>(SampledSpectrum(swl, cc_ior), swl);
            SP<GGXMicrofacet> microfacet_cc = make_shared<GGXMicrofacet>(make_float2(cc_roughness));
            UP<MicrofacetReflection> cc_refl = make_unique<MicrofacetReflection>(cc_weight * weight * cc_tint * front_factor, swl,
                                                                                 microfacet_cc);
            UP<CoatBxDFSet> cc_lobe = make_unique<CoatBxDFSet>(fresnel_cc, std::move(cc_refl));
            SampledSpectrum cc_albedo = cc_lobe->albedo(cos_theta) * front_factor;
            WeightedBxDFSet w_cc_lobe(cc_albedo.average(), std::move(cc_lobe));
            weight = layering_weight(cc_albedo, weight);
            lobes.push_back(std::move(w_cc_lobe));
        }
        {
            /// metallic
            Float metallic = metallic_.evaluate(it, swl).as_scalar();
            SP<FresnelF82Tint> fresnel_f82 = make_shared<FresnelF82Tint>(color, swl);
            fresnel_f82->init_from_F82(specular_tint);
            UP<MicrofacetReflection> metal_refl = make_unique<MicrofacetReflection>(weight * metallic,
                                                                                    swl, microfacet);
            WeightedBxDFSet metal_lobe(metallic * weight.average(),
                                       make_unique<MetallicBxDFSet>(fresnel_f82, std::move(metal_refl)));
            lobes.push_back(std::move(metal_lobe));
            weight *= (1.0f - metallic);
        }
        {
            /// transmission
            Float trans_weight = transmission_weight_.evaluate(it, swl).as_scalar();
            float_array etas = it.correct_eta(iors);
            Float eta = etas[0];
            auto fresnel = make_shared<FresnelDielectric>(SampledSpectrum{swl, eta}, swl);
            SampledSpectrum t_weight = trans_weight * weight;
            SP<Fresnel> fresnel_schlick = make_shared<FresnelGeneralizedSchlick>(schlick_F0_from_ior(eta) * specular_tint, etas, swl);
            UP<BxDFSet> dielectric = make_unique<DielectricBxDFSet>(fresnel_schlick, microfacet, color, false, SurfaceData::Glossy);
            WeightedBxDFSet trans_lobe(t_weight.average(), t_weight, std::move(dielectric));
            lobes.push_back(std::move(trans_lobe));
            weight *= (1.0f - trans_weight);
        }
        {
            /// specular
            Float f0 = schlick_F0_from_ior(ior);
            SP<Fresnel> fresnel_schlick = make_shared<FresnelGeneralizedSchlick>(f0 * specular_tint, iors, swl);
            UP<BxDFSet> spec_refl = make_unique<SpecularBxDFSet>(fresnel_schlick,
                                                                 make_unique<MicrofacetReflection>(weight, swl, microfacet));
            SampledSpectrum spec_refl_albedo = spec_refl->albedo(cos_theta);
            WeightedBxDFSet specular_lobe(spec_refl_albedo.average(), std::move(spec_refl));
            lobes.push_back(std::move(specular_lobe));
            weight = layering_weight(spec_refl_albedo, weight);
        }
        {
            /// diffuse
            SampledSpectrum diff_weight = color * weight;
            WeightedBxDFSet diffuse_lobe{diff_weight.average(), make_shared<DiffuseBxDFSet>(diff_weight, swl)};
            lobes.push_back(std::move(diffuse_lobe));
        }
        auto ret = make_unique<MultiBxDFSet>(std::move(lobes));
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, PrincipledBSDF)
VS_REGISTER_CURRENT_PATH(0, "vision-material-principled_bsdf.dll")