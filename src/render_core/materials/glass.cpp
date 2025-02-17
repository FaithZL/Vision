//
// Created by Zero on 05/10/2022.
//

#include <utility>

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/color/spd.h"

namespace vision {

class DielectricBxDFSet : public BxDFSet {
private:
    DCSP<Fresnel> fresnel_;
    MicrofacetReflection refl_;
    MicrofacetTransmission trans_;
    Bool dispersive_{};

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(fresnel_->type_hash(), refl_.type_hash(), trans_.type_hash());
    }

public:
    DielectricBxDFSet(const SP<Fresnel> &fresnel,
                         MicrofacetReflection refl,
                         MicrofacetTransmission trans,
                         Bool dispersive,
                         const Uint &flag)
        : fresnel_(fresnel),
          refl_(ocarina::move(refl)), trans_(ocarina::move(trans)),
          dispersive_(ocarina::move(dispersive)) {}
    VS_MAKE_BxDFSet_ASSIGNMENT(DielectricBxDFSet)
        [[nodiscard]] const SampledWavelengths *swl() const override { return &refl_.swl(); }
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override { return dispersive_; }
    [[nodiscard]] Bool splittable() const noexcept override { return true; }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override;
    [[nodiscard]] Uint flag() const noexcept override { return refl_.flags() | trans_.flags(); }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;
    [[nodiscard]] BSDFSample sample_delta_local(const Float3 &wo,
                                                TSampler &sampler) const noexcept override;
};

SampledSpectrum DielectricBxDFSet::albedo(const Float &cos_theta) const noexcept {
    SampledSpectrum F = fresnel_->evaluate(cos_theta);
    return F * refl_.albedo(cos_theta) + (1 - F) * trans_.albedo(cos_theta);
}


SampledDirection DielectricBxDFSet::sample_wi(const Float3 &wo, const Uint &flag,
                                                 TSampler &sampler) const noexcept {
    Float3 wh = refl_.microfacet()->sample_wh(wo, sampler->next_2d());
    Float d = dot(wo, wh);
    auto fresnel = fresnel_->clone();
    fresnel->correct_eta(wo.z);
    SampledDirection sd;
    SampledSpectrum frs = fresnel->evaluate(abs(d));
    Float uc = sampler->next_1d();
    $if(uc < frs[0]) {
//        wh = ocarina::select(d < 0.f, -wh, wh);
        sd.wi = reflect(wo, wh);
        sd.valid = same_hemisphere(wo, sd.wi);
    }
    $else {
        Float eta = fresnel->eta()[0];
        Bool valid = refract(wo, wh, eta, &sd.wi);
        sd.valid = valid && !same_hemisphere(wo, sd.wi);
    };
    return sd;
//    Float uc = sampler->next_1d();
//    auto fresnel = fresnel_->clone();
//    Float cos_theta_o = cos_theta(wo);
//    fresnel->correct_eta(cos_theta_o);
//    SampledSpectrum frs = fresnel->evaluate(abs_cos_theta(wo));
//    Float fr = frs[0];
//    SampledDirection ret;
//    $if(uc < fr) {
//        ret = refl_.sample_wi(wo, sampler->next_2d(), fresnel);
//    }
//    $else {
//        ret = trans_.sample_wi(wo, sampler->next_2d(), fresnel);
//    };
//    return ret;
}

BSDFSample DielectricBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                           TSampler &sampler) const noexcept {
    BSDFSample ret{*swl()};
    Float3 wh = refl_.microfacet()->sample_wh(wo, sampler->next_2d());
    Float d = dot(wo, wh);
    auto fresnel = fresnel_->clone();
    fresnel->correct_eta(wo.z);
    SampledDirection sd;
    SampledSpectrum frs = fresnel->evaluate(abs(d));
    Float uc = sampler->next_1d();
    ret.eta = fresnel->eta()[0];
    $if(uc < frs[0]) {
        Float3 wi = reflect(wo, wh);
        Bool valid = same_hemisphere(wi, wo);
        SampledSpectrum fr = refl_.microfacet()->BRDF(wo, wh, wi, frs);
        Float pdf = refl_.microfacet()->PDF_wi_reflection(wo, wh) * frs[0];
        ret.eval.f = fr;
        ret.eval.pdfs = pdf * cast<uint>(valid);
        ret.wi = wi;
        ret.eval.flags = BxDFFlag::GlossyRefl;
    }
    $else {
        Float3 wi;
        wh = face_forward(wh, wo);

      Bool valid = refract(wo, wh, ret.eta, &wi);
        SampledSpectrum tr = refl_.microfacet()->BTDF(wo, wh, wi, (1- frs[0]), ret.eta) * trans_.albedo(0);
        Float pdf = refl_.microfacet()->PDF_wi_transmission(wo, wh, wi, ret.eta) * (1 - frs[0]);
        valid = valid && !same_hemisphere(wi, wo);
        ret.eval.f = tr;
        ret.eval.pdfs = pdf * cast<uint>(valid);
        ret.wi = wi;
        ret.eval.flags = BxDFFlag::GlossyTrans;
    };
    return ret;
}

ScatterEval DielectricBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              MaterialEvalMode mode, const Uint &flag) const noexcept {
    ScatterEval ret{refl_.swl()};
    auto fresnel = fresnel_->clone();

    Bool reflect = same_hemisphere(wo, wi);

    fresnel->correct_eta(cos_theta(wo));

    Float eta = fresnel->eta()[0];

    $if(same_hemisphere(wo, wi)) {
        Float3 wh = normalize(wo + wi);
        SampledSpectrum frs = fresnel->evaluate(abs_dot(wh, wo));
        wh = face_forward(wh, make_float3(0, 0, 1));
        SampledSpectrum fr = refl_.microfacet()->BRDF(wo, wh, wi, frs);
        Float pdf = refl_.microfacet()->PDF_wi_reflection(wo, wh) * fr[0];
        ret.f = fr;
        ret.pdfs = pdf;
    }
    $else {
        Float3 wh = normalize(wo + eta * wi);
        SampledSpectrum frs = fresnel->evaluate(abs_dot(wh, wo));
        SampledSpectrum tr = refl_.microfacet()->BTDF(wo, wi, (1 - frs), eta);
        ret.f = tr * trans_.albedo(0);
        ret.pdfs = refl_.microfacet()->PDF_wi_transmission(wo, wh, wi, eta) * (1 - frs[0]);
    };
    return ret;
}


BSDFSample DielectricBxDFSet::sample_delta_local(const Float3 &wo, TSampler &sampler) const noexcept {
    BSDFSample ret{refl_.swl()};
    Float uc = sampler->next_1d();
    auto fresnel = fresnel_->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
    $if(uc < fr) {
        Float3 wi = wo;
        wi.xy() = -wi.xy();
        ret.wi = wi;
        ret.eval = refl_.evaluate(wo, wi, fresnel, All);
        ret.eval.pdfs *= fr;
    }
    $else {
        Float3 wi;
        Float3 n = face_forward(make_float3(0, 0, 1), wo);
        refract(wo, n, fresnel->eta()[0], &wi);
        ret.eval = trans_.evaluate(wo, wi, fresnel, All);
        ret.wi = wi;
        ret.eval.pdfs *= 1 - fr;
    };
    return ret;
}

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

//    using GlassBxDFSet = DielectricBxDFSetOld;
    using GlassBxDFSet = DielectricBxDFSet;

protected:
    VS_MAKE_MATERIAL_EVALUATOR(GlassBxDFSet)

public:
    GlassMaterial() = default;
    explicit GlassMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {
        color_.set(Slot::create_slot(desc.slot("color", make_float3(1.f), Albedo)));
        roughness_.set(Slot::create_slot(desc.slot("roughness", 0.01f)));
        anisotropic_.set(Slot::create_slot(desc.slot("anisotropic", 0.f)))->set_range(-1, 1);
        init_ior(desc);
        init_slot_cursor(&color_, &anisotropic_);
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
            eta_slot = SlotDesc(ShaderNodeDesc{ShaderNodeType::ESPD, "spd"}, 0);
            auto lst = SPD::to_list(*ior_curve(name));
            eta_slot.node.set_value("value", lst);
        } else {
            float lambda = rgb_spectrum_peak_wavelengths.x;
            float ior = (*ior_curve(name))(lambda);
            eta_slot = desc.slot("", ior);
        }
        ior_ = Slot::create_slot(eta_slot);
        ior_->set_range(1.003, 3.f);
        ior_->set_name("ior");
    }
    [[nodiscard]] bool is_dispersive() const noexcept override { return ior_->type() == ESPD; }

    [[nodiscard]] vector<PrecomputedLobeTable> precompute() const noexcept override {
        vector<PrecomputedLobeTable> ret;
        using namespace precompute;
        ret.push_back(precompute_lobe<DielectricReflectionBxDFSet>(make_uint3(DielectricReflectionBxDFSet::lut_res)));
        ret.push_back(precompute_lobe<DielectricRefractionBxDFSet>(make_uint3(DielectricReflectionBxDFSet::lut_res)));
        ret.push_back(precompute_lobe<DielectricReflectionInvEtaBxDFSet>(make_uint3(DielectricReflectionInvEtaBxDFSet::lut_res)));
        ret.push_back(precompute_lobe<DielectricRefractionInvEtaBxDFSet>(make_uint3(DielectricReflectionInvEtaBxDFSet::lut_res)));
        return ret;
    }

    void prepare() noexcept override { ior_->prepare(); }
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum color = color_.eval_albedo_spectrum(it, swl).sample;
        DynamicArray<float> iors = ior_.evaluate(it, swl);

        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl).as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);

        Float alpha_min = min(alpha.x, alpha.y);
        Uint flag = select(alpha_min < alpha_threshold_, SurfaceData::NearSpec, SurfaceData::Glossy);

        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y,
                                                     MaterialRegistry::instance().sample_visible());
        auto fresnel = make_shared<FresnelDielectric>(SampledSpectrum{iors}, swl);
        MicrofacetReflection refl(SampledSpectrum::one(swl.dimension()), swl, microfacet);
        MicrofacetTransmission trans(color, swl, microfacet);
        if (is_dispersive()) {
            $if(alpha_min < 0.008f) {
                swl.invalidation_secondary();
            };
        }
        return make_unique<GlassBxDFSet>(fresnel, ocarina::move(refl), ocarina::move(trans),
                                                 is_dispersive(), flag);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, GlassMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-glass.dll")