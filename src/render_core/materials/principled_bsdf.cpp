//
// Created by Zero on 2025/1/16.
//

#include <utility>
#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"
#include "ltc_sheen_table.h"
#include "base/scattering/precomputed_table.h"

namespace vision {

class FresnelSchlick : public Fresnel {
private:
    SampledSpectrum F0_;
    SampledSpectrum eta_;

public:
    FresnelSchlick(SampledSpectrum F0, SampledSpectrum eta,
                   const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)),
          eta_(std::move(eta)) {}
    FresnelSchlick(SampledSpectrum F0, const Float &eta,
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
    VS_MAKE_FRESNEL_ASSIGNMENT(FresnelSchlick)
};

/// reference https://tizianzeltner.com/projects/Zeltner2022Practical/
/// reference https://github.com/tizian/ltc-sheen
class SheenLTC : public Lobe {
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

class CoatLobe : public MicrofacetLobe {
public:
    using MicrofacetLobe::MicrofacetLobe;

    static constexpr float ior_lower = 1.003;
    static constexpr float ior_upper = 4.f;
    static constexpr const char *lut_name = "CoatLobe::lut";
    static constexpr uint lut_res = 32;
    static void prepare() {
        MaterialLut::instance().load_lut(lut_name, make_uint3(lut_res),
                                         PixelStorage::FLOAT1,
                                         addressof(CoatLobe_Table));
    }

    /// for precompute begin
    static constexpr const char *name = "CoatLobe";
    static UP<CoatLobe> create_for_precompute(const SampledWavelengths &swl) noexcept {
        SP<Fresnel> fresnel = make_shared<FresnelDielectric>(SampledSpectrum(swl, 1.5f), swl);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(make_float2(alpha_lower), true);
        UP<MicrofacetReflection> refl = make_unique<MicrofacetReflection>(SampledSpectrum::one(swl.dimension()),
                                                                          swl, microfacet);
        return make_unique<CoatLobe>(fresnel, ocarina::move(refl));
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

class SpecularLobe : public MicrofacetLobe {
public:
    using MicrofacetLobe::MicrofacetLobe;

    static constexpr const char *lut_name = "SpecularLobe::lut";
    static constexpr uint lut_res = 32;

    static void prepare() {
        MaterialLut::instance().load_lut(lut_name, make_uint3(lut_res),
                                         PixelStorage::FLOAT1,
                                         addressof(SpecularLobe_Table));
    }

    /// for precompute begin
    static constexpr const char *name = "SpecularLobe";
    static UP<SpecularLobe> create_for_precompute(const SampledWavelengths &swl) noexcept {
        SampledSpectrum f0 = SampledSpectrum(make_float3(0.04));
        SampledSpectrum f90 = SampledSpectrum(make_float3(1));
        SP<Fresnel> fresnel_schlick = make_shared<FresnelSchlick>(f0, 1.5f, swl);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(0.00f, 0.0f, true);
        UP<MicrofacetBxDF> bxdf = make_unique<MicrofacetReflection>(SampledSpectrum::one(swl), swl, microfacet);
        return make_unique<SpecularLobe>(fresnel_schlick, std::move(bxdf));
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
        SampledSpectrum f0 = fresnel<FresnelSchlick>()->F0();
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
public:
#define VS_MAKE_LOBE_TYPE(Type) E##Type
#define VS_MAKE_LOBE_NAME(Type) #Type
#define VS_MAKE_LOBE_TYPES(...)                                             \
    enum LobeType : uint8_t {                                               \
        MAP_LIST(VS_MAKE_LOBE_TYPE, ##__VA_ARGS__),                         \
        Count                                                               \
    };                                                                      \
    static constexpr std::array<const char *, LobeType::Count> LobeName = { \
        MAP_LIST(VS_MAKE_LOBE_NAME, ##__VA_ARGS__)};

    VS_MAKE_LOBE_TYPES(Sheen, Coat, Metallic, Trans, Spec, Diffuse)
#undef VS_MAKE_LOBE_TYPE
#undef VS_MAKE_LOBE_NAME
#undef VS_MAKE_LOBE_TYPES

private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(metallic)
    VS_MAKE_SLOT(ior)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(spec_tint)
    VS_MAKE_SLOT(anisotropic)
    VS_MAKE_SLOT(opcacity)

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

    SheenLTC::Mode sheen_mode_{SheenLTC::Approximate};
    std::array<bool, LobeType::Count> switches_{[] {
        std::array<bool, LobeType::Count> ret{};
        for (bool &elm : ret) { elm = true; }
        return ret;
    }()};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(LobeSet)

public:
    VS_MAKE_PLUGIN_NAME_FUNC
    PrincipledBSDF() = default;
    explicit PrincipledBSDF(const MaterialDesc &desc)
        : Material(desc) {}

    void initialize_(const vision::NodeDesc &node_desc) noexcept override {
        VS_CAST_DESC
        Material::initialize_(node_desc);
        INIT_SLOT(color, make_float3(1.f), Albedo);
        INIT_SLOT(metallic, 0.f, Number);
        INIT_SLOT(ior, 1.5f, Number).set_range(1.01, 20.f);
        INIT_SLOT(roughness, 0.5f, Number).set_range(0.0001f, 1.f);
        INIT_SLOT(spec_tint, make_float3(0.f), Albedo);
        INIT_SLOT(anisotropic, 0.f, Number);
        INIT_SLOT(opcacity, 1.f, Number).set_range(0.f, 1.f);

        INIT_SLOT(sheen_weight, 0.f, Number);
        INIT_SLOT(sheen_roughness, 0.5f, Number);
        INIT_SLOT(sheen_tint, make_float3(1.f), Albedo);

        INIT_SLOT(coat_weight, 0.f, Number);
        INIT_SLOT(coat_roughness, 0.2f, Number).set_range(0.0001f, 1.f);
        INIT_SLOT(coat_ior, 1.5f, Number).set_range(1.01, 4.f);
        INIT_SLOT(coat_tint, make_float3(1.f), Albedo);

        INIT_SLOT(subsurface_weight, 0.3f, Number);
        INIT_SLOT(subsurface_radius, make_float3(1.f), Number);
        INIT_SLOT(subsurface_scale, 0.2f, Number);

        INIT_SLOT(transmission_weight, 0.f, Number);

        init_slot_cursor(&color_, &transmission_weight_);
    }
    VS_HOTFIX_MAKE_RESTORE(Material, sheen_mode_, switches_)

    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        uint64_t ret = Material::compute_topology_hash();
        ret = hash64(ret, switches_);
        return ret;
    }

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        static vector<const char *> names = {"volume", "approximate"};
        widgets->combo("sheen mode", reinterpret_cast<int *>(addressof(sheen_mode_)), names);
        int colNum = 3;
        for (int i = 0; i < LobeName.size(); ++i) {
            widgets->check_box(LobeName[i], addressof(switches_[i]));
            if ((i + 1) % 3 != 0) {
                widgets->same_line();
            }
        }
        Material::render_sub_UI(widgets);
        update_switches();
    }
    void prepare() noexcept override {
        CoatLobe::prepare();
        SheenLTC::prepare();
        SpecularLobe::prepare();
        DielectricLobe::prepare();
        MetallicLobe::prepare();
    }

    void update_switches() noexcept {
        //        switches_[ESheen] = !sheen_weight_->near_zero();
        //        switches_[ECoat] = !coat_weight_->near_zero();
        //        switches_[EMetallic] = !metallic_->near_zero();
        //        switches_[ETrans] = (!metallic_->near_one()) && (!transmission_weight_->near_zero());
        //        switches_[ESpec] = (!metallic_->near_one()) && (!transmission_weight_->near_one());
        //        switches_[EDiffuse] = (!metallic_->near_one()) && (!transmission_weight_->near_one());
    }

    template<typename TLobe>
    [[nodiscard]] PrecomputedLobeTable precompute_lobe() const noexcept {
        return Material::precompute_lobe<TLobe>(make_uint3(TLobe::lut_res));
    }

    [[nodiscard]] vector<PrecomputedLobeTable> precompute() const noexcept override {
        vector<PrecomputedLobeTable> ret;
        ret.push_back(precompute_lobe<SpecularLobe>());
        ret.push_back(precompute_lobe<CoatLobe>());
        return ret;
    }
    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        LobeSet::Lobes lobes;
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
        if (switches_[ESheen]) {
            outline("principled sheen", [&] {
                SampledSpectrum sheen_tint = sheen_tint_.eval_albedo_spectrum(it, swl).sample;
                Float sheen_weight = sheen_weight_.evaluate(it, swl).as_scalar() * front_factor;
                Float sheen_roughness = sheen_roughness_.evaluate(it, swl).as_scalar();
                UP<SheenLTC> sheen_ltc = make_unique<SheenLTC>(sheen_mode_, cos_theta,
                                                               sheen_tint * sheen_weight * weight,
                                                               sheen_roughness, swl);
                SampledSpectrum sheen_albedo = sheen_ltc->albedo(cos_theta);
                WeightedLobe sheen_lobe(sheen_albedo.average(), std::move(sheen_ltc));
                lobes.push_back(std::move(sheen_lobe));
                weight = layering_weight(sheen_albedo, weight);
            });
        }
        if (switches_[ECoat]) {
            outline("principled coat", [&] {
                Float cc_weight = coat_weight_.evaluate(it, swl).as_scalar() * front_factor;
                Float cc_roughness = clamp(coat_roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
                cc_roughness = sqr(cc_roughness);
                Float cc_ior = coat_ior_.evaluate(it, swl).as_scalar();
                SampledSpectrum cc_tint = coat_tint_.eval_albedo_spectrum(it, swl).sample;
                SP<Fresnel> fresnel_cc = make_shared<FresnelDielectric>(SampledSpectrum(swl, cc_ior), swl);
                SP<GGXMicrofacet> microfacet_cc = make_shared<GGXMicrofacet>(make_float2(cc_roughness));
                UP<MicrofacetReflection> cc_refl = make_unique<MicrofacetReflection>(cc_weight * weight * cc_tint, swl,
                                                                                     microfacet_cc);
                UP<CoatLobe> cc_lobe = make_unique<CoatLobe>(fresnel_cc, std::move(cc_refl));
                SampledSpectrum cc_albedo = cc_lobe->albedo(cos_theta);
                WeightedLobe w_cc_lobe(cc_albedo.average(), std::move(cc_lobe));
                weight = layering_weight(cc_albedo, weight);
                lobes.push_back(std::move(w_cc_lobe));
            });
        }
        if (switches_[EMetallic]) {
            outline("principled metallic", [&] {
                Float metallic = metallic_.evaluate(it, swl).as_scalar() * front_factor;
                SP<FresnelF82Tint> fresnel_f82 = make_shared<FresnelF82Tint>(color, swl);
                fresnel_f82->init_from_F82(specular_tint);
                UP<MicrofacetReflection> metal_refl = make_unique<MicrofacetReflection>(weight * metallic,
                                                                                        swl, microfacet);
                WeightedLobe metal_lobe(metallic * weight.average(),
                                        make_unique<MetallicLobe>(fresnel_f82, std::move(metal_refl)));
                lobes.push_back(std::move(metal_lobe));
                weight *= (1.0f - metallic);
            });
        }
        if (switches_[ETrans]) {
            outline("principled transmission", [&] {
                Float trans_weight = transmission_weight_.evaluate(it, swl).as_scalar();
                float_array etas = it.correct_eta(iors);
                Float eta = etas[0];
                auto fresnel = make_shared<FresnelDielectric>(SampledSpectrum{swl, eta}, swl);
                SampledSpectrum t_weight = trans_weight * weight;
                SP<Fresnel> fresnel_schlick = make_shared<FresnelSchlick>(schlick_F0_from_ior(eta) * specular_tint, etas, swl);
                UP<Lobe> dielectric = make_unique<DielectricLobe>(fresnel_schlick, microfacet, color, false, SurfaceData::Glossy);
                WeightedLobe trans_lobe(t_weight.average(), t_weight.average(), std::move(dielectric));
                lobes.push_back(std::move(trans_lobe));
                weight *= (1.0f - trans_weight);
            });
        }
        if (switches_[ESpec]) {
            outline("principled specular", [&] {
                Float f0 = schlick_F0_from_ior(ior);
                SP<Fresnel> fresnel_schlick = make_shared<FresnelSchlick>(f0 * specular_tint, iors, swl);
                UP<Lobe> spec_refl = make_unique<SpecularLobe>(fresnel_schlick,
                                                               make_unique<MicrofacetReflection>(weight, swl, microfacet));
                SampledSpectrum spec_refl_albedo = spec_refl->albedo(cos_theta);
                WeightedLobe specular_lobe(spec_refl_albedo.average(), std::move(spec_refl));
                lobes.push_back(std::move(specular_lobe));
                weight = layering_weight(spec_refl_albedo, weight);
            });
        }
        if (switches_[EDiffuse]) {
            outline("principled diffuse", [&] {
                SampledSpectrum diff_weight = color * weight * front_factor;
                WeightedLobe diffuse_lobe{diff_weight.average(), make_shared<DiffuseLobe>(diff_weight, swl)};
                lobes.push_back(std::move(diffuse_lobe));
            });
        }
        UP<LobeSet> ret = make_unique<LobeSet>(std::move(lobes));
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, PrincipledBSDF)
VS_REGISTER_CURRENT_PATH(0, "vision-material-principled_bsdf.dll")