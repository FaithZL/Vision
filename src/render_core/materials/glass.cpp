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
    Bool dispersive_{};
    DCSP<Microfacet<D>> microfacet_;
    SampledSpectrum kt_{};
    Uint flag_{};

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(fresnel_->type_hash());
    }

public:
    DielectricBxDFSet(const SP<Fresnel> &fresnel, const SP<Microfacet<D>> &microfacet,
                      SampledSpectrum color, Bool dispersive, Uint flag)
        : fresnel_(fresnel), microfacet_(microfacet),
          kt_(std::move(color)), dispersive_(ocarina::move(dispersive)),
          flag_(std::move(flag)) {}
    VS_MAKE_BxDFSet_ASSIGNMENT(DielectricBxDFSet)
        [[nodiscard]] const SampledWavelengths *swl() const override { return fresnel_->swl(); }
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override { return dispersive_; }
    [[nodiscard]] Bool splittable() const noexcept override { return true; }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override;
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                             MaterialEvalMode mode, const Uint &flag) const noexcept override;
    [[nodiscard]] ScatterEval evaluate_reflection(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                  const SampledSpectrum &fr, MaterialEvalMode mode) const noexcept;
    [[nodiscard]] ScatterEval evaluate_transmission(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                    const SampledSpectrum &eta, MaterialEvalMode mode) const noexcept;
    [[nodiscard]] Uint flag() const noexcept override { return flag_; }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;
};

SampledSpectrum DielectricBxDFSet::albedo(const Float &cos_theta) const noexcept {
    return kt_;
}

ScatterEval DielectricBxDFSet::evaluate_reflection(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                   const SampledSpectrum &F, MaterialEvalMode mode) const noexcept {
    ScatterEval se{*swl()};
    se.f = microfacet_->BRDF(wo, wh, wi, F);
    se.pdfs = microfacet_->PDF_wi_reflection(wo, wh) * F[0];
    se.flags = BxDFFlag::GlossyRefl;
    return se;
}

ScatterEval DielectricBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wh,
                                              const Float3 &wi, MaterialEvalMode mode,
                                              const Uint &flag) const noexcept {
    ScatterEval ret{*swl()};
    auto fresnel = fresnel_->clone();
    Bool reflect = same_hemisphere(wo, wi);
    fresnel->correct_eta(cos_theta(wo));
    Float eta_p = ocarina::select(reflect, 1.f, fresnel->eta()[0]);
    $if(reflect) {
        SampledSpectrum frs = fresnel->evaluate(abs_dot(wh, wo));
        ret = evaluate_reflection(wo, wh, wi, frs, mode);
    }
    $else {
        SampledSpectrum frs = fresnel->evaluate(abs_dot(wh, wo));
        SampledSpectrum tr = microfacet_->BTDF(wo, wi, (1 - frs), eta_p);
        ret.f = tr * kt_;
        ret.pdfs = microfacet_->PDF_wi_transmission(wo, wh, wi, eta_p) * (1 - frs[0]);
        ret.flags = BxDFFlag::GlossyTrans;
    };
    return ret;
}

SampledDirection DielectricBxDFSet::sample_wi(const Float3 &wo, const Uint &flag,
                                              TSampler &sampler) const noexcept {
    Float3 wh = microfacet_->sample_wh(wo, sampler->next_2d());
    Float d = dot(wo, wh);
    auto fresnel = fresnel_->clone();
    fresnel->correct_eta(wo.z);
    SampledDirection sd;
    SampledSpectrum frs = fresnel->evaluate(abs(d));
    Float uc = sampler->next_1d();
    $if(uc < frs[0]) {
        sd.wi = reflect(wo, wh);
        sd.wh = wh;
    }
    $else {
        Float eta = fresnel->eta()[0];
        Bool valid = refract(wo, wh, eta, &sd.wi);
        sd.valid = valid;
        sd.wh = wh;
    };
    return sd;
}

BSDFSample DielectricBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                           TSampler &sampler) const noexcept {
    BSDFSample ret{*swl()};
    Float3 wh = microfacet_->sample_wh(wo, sampler->next_2d());
    Float d = dot(wo, wh);
    auto fresnel = fresnel_->clone();
    fresnel->correct_eta(wo.z);
    SampledSpectrum frs = fresnel->evaluate(abs(d));
    Float uc = sampler->next_1d();
    ret.eta = fresnel->eta()[0];
    $if(uc < frs[0]) {
        Float3 wi = reflect(wo, wh);
        ret.eval = evaluate_reflection(wo, wh, wi, frs, MaterialEvalMode::All);
        ret.wi = wi;
    }
    $else {
        Float3 wi;
        wh = face_forward(wh, wo);

        Bool valid = refract(wo, wh, ret.eta, &wi);
        SampledSpectrum tr = microfacet_->BTDF(wo, wh, wi, (1 - frs[0]), ret.eta) * kt_;
        Float pdf = microfacet_->PDF_wi_transmission(wo, wh, wi, ret.eta) * (1 - frs[0]);
        ret.eval.f = tr;
        ret.eval.pdfs = pdf * cast<uint>(valid);
        ret.wi = wi;
        ret.eval.flags = BxDFFlag::GlossyTrans;
    };
    return ret;
}

ScatterEval DielectricBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              MaterialEvalMode mode, const Uint &flag) const noexcept {
    ScatterEval ret{*swl()};
    auto fresnel = fresnel_->clone();

    Bool reflect = same_hemisphere(wo, wi);

    fresnel->correct_eta(cos_theta(wo));

    Float eta_p = ocarina::select(reflect, 1.f, fresnel->eta()[0]);
    Float3 wh = normalize(wo + eta_p * wi);
    $if(reflect) {
        SampledSpectrum frs = fresnel->evaluate(abs_dot(wh, wo));
        ret = evaluate_reflection(wo, wh, wi, frs, mode);
    }
    $else {
        SampledSpectrum frs = fresnel->evaluate(abs_dot(wh, wo));
        SampledSpectrum tr = microfacet_->BTDF(wo, wi, (1 - frs), eta_p);
        ret.f = tr * kt_;
        ret.pdfs = microfacet_->PDF_wi_transmission(wo, wh, wi, eta_p) * (1 - frs[0]);
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
        return make_unique<DielectricBxDFSet>(fresnel, microfacet, color, is_dispersive(), flag);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, GlassMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-glass.dll")