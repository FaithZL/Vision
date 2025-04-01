//
// Created by Zero on 05/10/2022.
//

#include <utility>

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/color/spd.h"

namespace vision {

class DielectricPrecompute : public DielectricLobe {
public:
    using DielectricLobe::DielectricLobe;
    [[nodiscard]] bool compensate() const noexcept override { return false; }
    [[nodiscard]] SampledSpectrum integral_albedo(const Float3 &wo, TSampler &sampler,
                                                  const Uint &sample_num) const noexcept override {
        SampledSpectrum ret = SampledSpectrum::zero(*swl());

        $for(i, sample_num) {
            BSDFSample bs = sample_local(wo, BxDFFlag::All, sampler, Importance);
            ScatterEval se = bs.eval;
            $if(se.pdf() > 0) {
                SampledSpectrum r = se.throughput() * abs_cos_theta(bs.wi);
                ret[0] += r[0];
                $if(same_hemisphere(bs.wi, wo)) {
                    ret[1] += r[0];
                };
            };
        };
        return ret / sample_num;
    }

    void from_ratio_x(const ocarina::Float &x) noexcept override {
        Float a = ocarina::clamp(sqr(x), alpha_lower, alpha_upper);
        microfacet_->set_alpha_x(a);
        microfacet_->set_alpha_y(a);
    }
};

class DielectricLobePrecompute : public DielectricPrecompute {
public:
    using DielectricPrecompute::DielectricPrecompute;

    /// for precompute begin
    static UP<DielectricLobePrecompute> create_for_precompute(const SampledWavelengths &swl) noexcept {
        SP<Fresnel> fresnel = make_shared<FresnelDielectric>(SampledSpectrum(swl, 1.5f), swl);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(make_float2(0.001f), true);
        return make_unique<DielectricLobePrecompute>(fresnel, microfacet, SampledSpectrum::one(3), false, BxDFFlag::Glossy);
    }
    static constexpr const char *name = "DielectricLobe";
    void from_ratio_z(ocarina::Float z) noexcept override {
        Float ior = lerp(z, ior_lower, ior_upper);
        fresnel_->set_eta(SampledSpectrum(*swl(), ior));
    }
    /// for precompute end
};

class DielectricLobeInvPrecompute : public DielectricPrecompute {
public:
    using DielectricPrecompute::DielectricPrecompute;
    /// for precompute begin
    static UP<DielectricLobeInvPrecompute> create_for_precompute(const SampledWavelengths &swl) noexcept {
        SP<Fresnel> fresnel = make_shared<FresnelDielectric>(SampledSpectrum(swl, 1.5f), swl);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(make_float2(0.001f), true);
        return make_unique<DielectricLobeInvPrecompute>(fresnel, microfacet, SampledSpectrum::one(3), false, BxDFFlag::Glossy);
    }
    static constexpr const char *name = "DielectricInvLobe";
    void from_ratio_z(ocarina::Float z) noexcept override {
        Float ior = lerp(z, ior_lower, ior_upper);
        fresnel_->set_eta(SampledSpectrum(*swl(), rcp(ior)));
    }
    /// for precompute end
};

class IORCurve {
public:
    [[nodiscard]] virtual Float eta(const Float &lambda) const noexcept = 0;
    [[nodiscard]] virtual float eta(float lambda) const noexcept = 0;
    [[nodiscard]] virtual SampledSpectrum eta(const SampledWavelengths &swl) const noexcept = 0;

    [[nodiscard]] float operator()(float lambda) const noexcept {
        return eta(lambda);
    }
};

#define VS_IOR_CURVE_COMMON                                                                    \
    [[nodiscard]] Float eta(const Float &lambda) const noexcept override {                     \
        return _eta(lambda);                                                                   \
    }                                                                                          \
    [[nodiscard]] float eta(float lambda) const noexcept override {                            \
        return _eta(lambda);                                                                   \
    }                                                                                          \
    [[nodiscard]] SampledSpectrum eta(const SampledWavelengths &swl) const noexcept override { \
        SampledSpectrum ret{swl.dimension()};                                                  \
        for (uint i = 0; i < swl.dimension(); ++i) {                                           \
            ret[i] = eta(swl.lambda(i));                                                       \
        }                                                                                      \
        return ret;                                                                            \
    }

class BK7 : public IORCurve {
private:
    [[nodiscard]] auto _eta(auto lambda) const noexcept {
        using ocarina::sqr;
        using ocarina::sqrt;
        lambda = lambda / 1000.f;
        auto f = 1.03961212f * sqr(lambda) / (sqr(lambda) - 0.00600069867f) +
                 0.231792344f * sqr(lambda) / (sqr(lambda) - 0.0200179144f) +
                 1.01046945f * sqr(lambda) / (sqr(lambda) - 103.560653f);
        return sqrt(f + 1);
    }

public:
    VS_IOR_CURVE_COMMON
};

class LASF9 : public IORCurve {
private:
    [[nodiscard]] auto _eta(auto lambda) const noexcept {
        using ocarina::sqr;
        using ocarina::sqrt;
        lambda = lambda / 1000.f;
        auto f = 2.00029547f * sqr(lambda) / (sqr(lambda) - 0.0121426017f) +
                 0.298926886f * sqr(lambda) / (sqr(lambda) - 0.0538736236f) +
                 1.80691843f * sqr(lambda) / (sqr(lambda) - 156.530829f);
        return sqrt(f + 1);
    }

public:
    VS_IOR_CURVE_COMMON
};

#undef VS_IOR_CURVE_COMMON

[[nodiscard]] static IORCurve *ior_curve(string name) noexcept {
    using Map = map<string, UP<IORCurve>>;
    static Map curve_map = [&]() {
        Map ret;
#define VS_MAKE_GLASS_IOR_CURVE(name) \
    ret[#name] = make_unique<name>();
        VS_MAKE_GLASS_IOR_CURVE(BK7)
        VS_MAKE_GLASS_IOR_CURVE(LASF9)
#undef VS_MAKE_GLASS_IOR_CURVE
        return ret;
    }();
    if (curve_map.find(name) == curve_map.end()) {
        if (name.empty()) {
            return nullptr;
        }
        name = "BK7";
    }
    return curve_map[name].get();
}

//    "type" : "glass",
//    "param" : {
//        "material_name" : "BK7",
//        "color" : {
//            "channels" : "xyz",
//            "node" : [
//                1,
//                1,
//                1
//            ]
//        },
//        "roughness" : [0.001, 0.001]
//    }
class GlassMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(ior)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(anisotropic)
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    VS_MAKE_MATERIAL_EVALUATOR(DielectricLobe)

public:
    GlassMaterial() = default;
    explicit GlassMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {}

    void initialize_(const vision::NodeDesc &node_desc) noexcept override {
        VS_CAST_DESC
        INIT_SLOT(color, make_float3(1.f), Albedo);
        INIT_SLOT(roughness, 0.5f, Number).set_range(0.0001f, 1.f);
        INIT_SLOT(anisotropic, 0.f, Number).set_range(-1, 1);
        init_ior(desc);
        init_slot_cursor(&color_, &anisotropic_);
    }
    template<typename TLobe>
    [[nodiscard]] PrecomputedLobeTable precompute_lobe() const noexcept {
        return Material::precompute_lobe<TLobe, 2>(make_uint3(TLobe::lut_res));
    }

    [[nodiscard]] vector<PrecomputedLobeTable> precompute() const noexcept override {
        vector<PrecomputedLobeTable> ret;
        ret.push_back(precompute_lobe<DielectricLobePrecompute>());
        ret.push_back(precompute_lobe<DielectricLobeInvPrecompute>());
        return ret;
    }

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->input_float("alpha_threshold", &alpha_threshold_, 0.001, 0.002);
        Material::render_sub_UI(widgets);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(Material, remapping_roughness_, alpha_threshold_)
    void init_ior(const MaterialDesc &desc) noexcept {
        auto name = desc["material_name"].as_string();
        SlotDesc eta_slot;
        if (name.empty()) {
            eta_slot = desc.slot("ior", 1.5f);
        } else if (spectrum()->is_complete()) {
            eta_slot = SlotDesc(ShaderNodeDesc{ShaderNodeTag::ESPD, "spd"}, 0);
            auto lst = SPD::to_list(*ior_curve(name));
            eta_slot.node.set_value("value", lst);
        } else {
            float lambda = rgb_spectrum_peak_wavelengths.x;
            float ior = (*ior_curve(name))(lambda);
            eta_slot = desc.slot("", ior);
        }
        ior_ = Slot::create_slot(eta_slot);
        ior_->set_range(DielectricLobe::ior_lower, DielectricLobe::ior_upper);
        ior_->set_name("ior");
    }
    [[nodiscard]] bool is_dispersive() const noexcept override { return ior_->node_tag() == ESPD; }
    void prepare() noexcept override {
        ior_->prepare();
        DielectricLobe::prepare();
    }

    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum color = color_.eval_albedo_spectrum(it, swl).sample;
        DynamicArray<float> iors = ior_.evaluate(it, swl);
        iors = it.correct_eta(iors);
        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.01f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl).as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);

        Float alpha_min = min(alpha.x, alpha.y);
        Uint flag = select(alpha_min < alpha_threshold_, SurfaceData::NearSpec, SurfaceData::Glossy);

        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelDielectric>(SampledSpectrum{iors}, swl);
        return make_unique<DielectricLobe>(fresnel, microfacet, color, is_dispersive(), flag);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, GlassMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-glass.dll")