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
public:
    static constexpr float ior_lower = 1.5f;
    static constexpr float ior_upper = 3.f;
    static constexpr const char *lut_name = "DielectricBxDFSet::lut";
    static constexpr uint lut_res = 16;

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
    [[nodiscard]] ScatterEval evaluate_reflection(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                  const SampledSpectrum &fr, MaterialEvalMode mode) const noexcept;
    [[nodiscard]] ScatterEval evaluate_transmission(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                    const SampledSpectrum &F, const SampledSpectrum &eta,
                                                    MaterialEvalMode mode) const noexcept;
    [[nodiscard]] ScatterEval evaluate_impl(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                            const SP<const Fresnel> &fresnel, MaterialEvalMode mode) const noexcept;
    [[nodiscard]] Float refl_prob(const SampledSpectrum &F) const noexcept {
        SampledSpectrum T = 1 - F;
        SampledSpectrum total = T * kt_ + F;
        return F.average() / total.average();
    }

    [[nodiscard]] Float trans_prob(const SampledSpectrum &F) const noexcept {
        return 1 - refl_prob(F);
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
                                             MaterialEvalMode mode, const Uint &flag, Float *eta) const noexcept override;
    [[nodiscard]] Uint flag() const noexcept override { return flag_; }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;

    /// for precompute begin
    static UP<DielectricBxDFSet> create_for_precompute(const SampledWavelengths &swl) noexcept {
        SP<Fresnel> fresnel = make_shared<FresnelDielectric>(SampledSpectrum(swl, 1.5f), swl);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(make_float2(0.001f), true);
        return make_unique<DielectricBxDFSet>(fresnel, microfacet, SampledSpectrum::one(3), false, BxDFFlag::Glossy);
    }
    static constexpr const char *name = "DielectricBxDFSet";
    void from_ratio_z(ocarina::Float z) noexcept override {
        Float ior = lerp(z, ior_lower, ior_upper);
        fresnel_->set_eta(SampledSpectrum(*swl(), ior));
    }

    void from_ratio_x(const ocarina::Float &x) noexcept override {
        microfacet_->set_alpha_x(ocarina::clamp(x, alpha_lower, alpha_upper));
        microfacet_->set_alpha_y(ocarina::clamp(x, alpha_lower, alpha_upper));
    }

    Float4 precompute_with_radio2(const ocarina::Float3 &ratio, vision::TSampler &sampler, const ocarina::Uint &sample_num) noexcept {
        from_ratio_x(ratio.x);
        Float3 wo = from_ratio_y(ratio.y);
        from_ratio_z(ratio.z);
        return precompute_albedo1(wo, sampler, sample_num);
    }

    Float4 precompute_albedo1(const ocarina::Float3 &wo, vision::TSampler &sampler, const ocarina::Uint &sample_num) noexcept {
//        SampledSpectrum ret = SampledSpectrum::zero(3);
        Float4 ret;
        Uint count = 0;
        Float reflection = 0;
        Float trans = 0;
        Float total = 0;

        $for(i, sample_num) {
            BSDFSample bs = sample_local(wo, BxDFFlag::All, sampler);
            ScatterEval se = bs.eval;
            $if(se.pdf() > 0) {
                auto r = se.throughput() * abs_cos_theta(bs.wi);
                count += 1;
                total += r.average();

                $if(same_hemisphere(wo, bs.wi)) {
                    reflection += r.average();
                } $else {
                    trans += r.average();
                };
            };
        };
        Float fr = fresnel_->evaluate(cos_theta(wo), 0);
        ret = make_float4(total, reflection, trans, 0.f);
        ret = ret / sample_num;
        ret.w = fr;
        return ret;
    }
    /// for precompute end

    [[nodiscard]] Float to_ratio_z() const noexcept override {
        Float ior = fresnel_->eta().average();
        return inverse_lerp(ior, ior_lower, ior_upper);
    }
    Float to_ratio_x() const noexcept override {
        Float ax = microfacet_->alpha_x();
        Float ay = microfacet_->alpha_y();
        return ocarina::clamp(ocarina::sqrt(ax * ay), alpha_lower, alpha_upper);
    }
};

SampledSpectrum DielectricBxDFSet::albedo(const Float &cos_theta) const noexcept {
    SP<Fresnel> fresnel = fresnel_->clone();
    fresnel->correct_eta(cos_theta);
    SampledSpectrum eta = fresnel->eta();
    SampledSpectrum F = fresnel->evaluate(abs(cos_theta));
    return kt_ * (1 - F) + F;
}

ScatterEval DielectricBxDFSet::evaluate_impl(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                             const SP<const Fresnel> &fresnel, MaterialEvalMode mode) const noexcept {
    ScatterEval ret{*swl()};
    Bool reflect = same_hemisphere(wo, wi);
    SampledSpectrum F = fresnel->evaluate(abs_dot(wh, wo));
    $if(reflect) {
        ret = evaluate_reflection(wo, wh, wi, F, mode);
    }
    $else {
        ret = evaluate_transmission(wo, wh, wi, F, fresnel->eta(), mode);
    };
    return ret;
}

ScatterEval DielectricBxDFSet::evaluate_reflection(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                   const SampledSpectrum &F, MaterialEvalMode mode) const noexcept {
    ScatterEval se{*swl()};
    if (BxDF::match_F(mode)) {
        se.f = microfacet_->BRDF(wo, wh, wi, F);
    }
    if (BxDF::match_PDF(mode)) {
        se.pdfs = microfacet_->PDF_wi_reflection(wo, wh) * refl_prob(F);
    }
    se.flags = BxDFFlag::GlossyRefl;
    return se;
}

ScatterEval DielectricBxDFSet::evaluate_transmission(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                     const SampledSpectrum &F, const SampledSpectrum &eta,
                                                     MaterialEvalMode mode) const noexcept {
    ScatterEval se{*swl()};
    Float3 new_wh = face_forward(wh, wo);
    if (BxDF::match_F(mode)) {
        SampledSpectrum tr = microfacet_->BTDF(wo, wi, (1 - F), eta[0]);
        se.f = tr * kt_ * sqr(eta);
    }
    if (BxDF::match_PDF(mode)) {
        se.pdfs = microfacet_->PDF_wi_transmission(wo, new_wh, wi, eta[0]) * trans_prob(F);
    }
    se.flags = BxDFFlag::GlossyTrans;
    return se;
}

ScatterEval DielectricBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              MaterialEvalMode mode, const Uint &flag) const noexcept {
    SP<Fresnel> fresnel = fresnel_->clone();
    fresnel->correct_eta(cos_theta(wo));
    Bool reflect = same_hemisphere(wo, wi);
    Float eta_p = ocarina::select(reflect, 1.f, fresnel->eta()[0]);
    Float3 wh = normalize(wo + eta_p * wi);
    return evaluate_impl(wo, wh, wi, fresnel, mode);
}

ScatterEval DielectricBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wh,
                                              const Float3 &wi, MaterialEvalMode mode,
                                              const Uint &flag, Float *eta) const noexcept {
    SP<Fresnel> fresnel = fresnel_->clone();
    fresnel->correct_eta(cos_theta(wo));
    if (eta) { *eta = fresnel->eta()[0]; }
    return evaluate_impl(wo, wh, wi, fresnel, mode);
}

SampledDirection DielectricBxDFSet::sample_wi(const Float3 &wo, const Uint &flag,
                                              TSampler &sampler) const noexcept {
    Float3 wh = microfacet_->sample_wh(wo, sampler->next_2d());
    Float d = dot(wo, wh);
    auto fresnel = fresnel_->clone();
    fresnel->correct_eta(wo.z);
    SampledDirection sd;
    SampledSpectrum F = fresnel->evaluate(abs(d));
    Float uc = sampler->next_1d();
    $if(uc < refl_prob(F)) {
        sd.wi = reflect(wo, wh);
        sd.wh = wh;
        sd.valid = same_hemisphere(wo, sd.wi);
    }
    $else {
        Float eta = fresnel->eta()[0];
        Bool valid = refract(wo, wh, eta, &sd.wi);
        sd.valid = valid && !same_hemisphere(wo, sd.wi);
        sd.wh = wh;
    };
    return sd;
}

BSDFSample DielectricBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                           TSampler &sampler) const noexcept {
    return BxDFSet::sample_local(wo, flag, sampler);
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

protected:
    VS_MAKE_MATERIAL_EVALUATOR(DielectricBxDFSet)

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
    template<typename TLobe>
    [[nodiscard]] PrecomputedLobeTable precompute_lobe(uint3 res) const noexcept {
        Device &device = Global::instance().device();
        Stream stream = device.create_stream();
        TSampler &sampler = get_sampler();

        Buffer<float4> buffer = device.create_buffer<float4>(res.x * res.y * res.z);

        Kernel kernel = [&](Uint sample_num) {
            sampler->load_data();
            sampler->start(dispatch_idx().xy(), 0, 0);
            SampledWavelengths swl = spectrum()->sample_wavelength(sampler);
            Float3 ratio = make_float3(dispatch_idx()) / make_float3(dispatch_dim() - 1);
            UP<TLobe> lobe = TLobe::create_for_precompute(swl);
            Float4 result = lobe->precompute_with_radio2(ratio, sampler, sample_num);
            buffer.write(dispatch_id(), result);
        };

        PrecomputedLobeTable ret;
        ret.name = TLobe::name;
        ret.type = Type::of<float4>();
        ret.data.resize(buffer.size() * ret.type->dimension());
        ret.res = res;
        Clock clk;
        clk.start();
        OC_INFO_FORMAT("start precompute albedo of {}", ret.name);
        auto shader = device.compile(kernel);
        stream << shader(precompute_sample_num).dispatch(res)
               << buffer.download(ret.data.data())
               << Env::printer().retrieve()
               << synchronize() << commit();
        clk.end();
        ret.elapsed_time = clk.elapse_s();
        OC_INFO_FORMAT("precompute albedo of {} took {:.3f} s", ret.name, ret.elapsed_time);

        return ret;
    }
    template<typename TLobe>
    [[nodiscard]] PrecomputedLobeTable precompute_lobe() const noexcept {
        return precompute_lobe<TLobe>(make_uint3(TLobe::lut_res));
    }

    [[nodiscard]] vector<PrecomputedLobeTable> precompute() const noexcept override {
        vector<PrecomputedLobeTable> ret;
        ret.push_back(precompute_lobe<DielectricBxDFSet>());
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
            eta_slot = SlotDesc(ShaderNodeDesc{ShaderNodeType::ESPD, "spd"}, 0);
            auto lst = SPD::to_list(*ior_curve(name));
            eta_slot.node.set_value("value", lst);
        } else {
            float lambda = rgb_spectrum_peak_wavelengths.x;
            float ior = (*ior_curve(name))(lambda);
            eta_slot = desc.slot("", ior);
        }
        ior_ = Slot::create_slot(eta_slot);
        ior_->set_range(DielectricBxDFSet::ior_lower, DielectricBxDFSet::ior_upper);
        ior_->set_name("ior");
    }
    [[nodiscard]] bool is_dispersive() const noexcept override { return ior_->type() == ESPD; }
    void prepare() noexcept override { ior_->prepare(); }

    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum color = color_.eval_albedo_spectrum(it, swl).sample;
        DynamicArray<float> iors = ior_.evaluate(it, swl);

        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.01f, 1.f);
        Float anisotropic = ocarina::clamp(anisotropic_.evaluate(it, swl).as_scalar(), -0.9f, 0.9f);

        roughness = remapping_roughness_ ? roughness_to_alpha(roughness) : roughness;
        Float2 alpha = calculate_alpha<D>(roughness, anisotropic);

        Float alpha_min = min(alpha.x, alpha.y);
        Uint flag = select(alpha_min < alpha_threshold_, SurfaceData::NearSpec, SurfaceData::Glossy);

        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y,
                                                     MaterialRegistry::instance().sample_visible());
        auto fresnel = make_shared<FresnelDielectric>(SampledSpectrum{iors}, swl);
        return make_unique<DielectricBxDFSet>(fresnel, microfacet, color, is_dispersive(), flag);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, GlassMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-glass.dll")