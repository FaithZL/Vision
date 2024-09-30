//
// Created by Zero on 05/10/2022.
//

#include <utility>

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/color/spd.h"

namespace vision {

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
        : BxDFSet(flag),
          fresnel_(fresnel),
          refl_(ocarina::move(refl)), trans_(ocarina::move(trans)),
          dispersive_(ocarina::move(dispersive)) {}
    VS_MAKE_BxDFSet_ASSIGNMENT(DielectricBxDFSet)
        [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return refl_.albedo(wo); }
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override { return dispersive_; }
    [[nodiscard]] Bool splittable() const noexcept override { return true; }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override {
        ScatterEval ret{refl_.swl().dimension()};
        auto fresnel = fresnel_->clone();
        Float cos_theta_o = cos_theta(wo);
        fresnel->correct_eta(cos_theta_o);
        Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
        $if(same_hemisphere(wo, wi)) {
            ret = refl_.evaluate(wo, wi, fresnel, mode);
            ret.pdf() *= fr;
        }
        $else {
            ret = trans_.evaluate(wo, wi, fresnel, mode);
            ret.pdf() *= 1 - fr;
        };
        return ret;
    }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override {
        Float uc = sampler->next_1d();
        auto fresnel = fresnel_->clone();
        Float cos_theta_o = cos_theta(wo);
        fresnel->correct_eta(cos_theta_o);
        Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
        SampledDirection ret;
        $if(uc < fr) {
            ret = refl_.sample_wi(wo, sampler->next_2d(), fresnel);
            ret.pdf = fr;
        }
        $else {
            ret = trans_.sample_wi(wo, sampler->next_2d(), fresnel);
            ret.pdf = 1 - fr;
        };
        return ret;
    }

    [[nodiscard]] BSDFSample sample_delta_local(const Float3 &wo, TSampler &sampler) const noexcept override {
        BSDFSample ret{refl_.swl().dimension()};
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
            ret.eval.pdf() *= fr;
        }
        $else {
            Float3 wi;
            Float3 n = face_forward(make_float3(0,0,1), wo);
            refract(wo, n, fresnel->eta()[0], &wi);
            ret.eval = trans_.evaluate(wo, wi, fresnel, All);
            ret.wi = wi;
            ret.eval.pdf() *= 1 - fr;
        };
        return ret;
    }

    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override {
        BSDFSample ret{refl_.swl().dimension()};
        Float uc = sampler->next_1d();
        auto fresnel = fresnel_->clone();
        Float cos_theta_o = cos_theta(wo);
        fresnel->correct_eta(cos_theta_o);
        Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
        $if(uc < fr) {
            ret = refl_.sample(wo, sampler, fresnel);
            ret.eval.pdf() *= fr;
        }
        $else {
            ret = trans_.sample(wo, sampler, fresnel);
            ret.eval.pdf() *= 1 - fr;
        };
        return ret;
    }
};

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
    VS_MAKE_SLOT(color);
    VS_MAKE_SLOT(ior);
    VS_MAKE_SLOT(roughness);
    bool remapping_roughness_{true};
    float alpha_threshold_{0.022};

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<DielectricBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    GlassMaterial() = default;
    explicit GlassMaterial(const MaterialDesc &desc)
        : Material(desc),
          remapping_roughness_(desc["remapping_roughness"].as_bool(true)) {
        color_.set(Slot::create_slot(desc.slot("color", make_float3(1.f), Albedo)));
        roughness_.set(Slot::create_slot(desc.slot("roughness", make_float2(0.01f))));
        init_ior(desc);
        init_slot_cursor(&color_, 3);
    }
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->input_float("alpha_threshold", &alpha_threshold_, 0.001, 0.002);
        Material::render_sub_UI(widgets);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
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
        ior_->set_name("ior");
    }
    [[nodiscard]] bool is_dispersive() const noexcept override { return ior_->type() == ESPD; }
    void prepare() noexcept override { ior_->prepare(); }
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum color = color_.eval_albedo_spectrum(it, swl).sample;
        DynamicArray<float> iors = ior_.evaluate(it, swl);

        Float2 alpha = roughness_.evaluate(it, swl).as_vec2();
        alpha = remapping_roughness_ ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        Float alpha_min = min(alpha.x, alpha.y);
        Uint flag = select(alpha_min < alpha_threshold_, SurfaceData::NearSpec, SurfaceData::Glossy);

        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelDielectric>(SampledSpectrum{iors},
                                                      swl, pipeline());
        MicrofacetReflection refl(SampledSpectrum(swl.dimension(), 1.f), swl, microfacet);
        MicrofacetTransmission trans(color, swl, microfacet);

        return make_unique<DielectricBxDFSet>(fresnel, ocarina::move(refl), ocarina::move(trans),
                                              is_dispersive(), flag);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, GlassMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-glass.dll")