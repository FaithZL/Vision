//
// Created by Zero on 28/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

inline namespace disney {

class Diffuse : public BxDF {
private:
    Float3 _color;

public:
    Diffuse() = default;
    explicit Diffuse(Float3 color)
        : BxDF(BxDFFlag::DiffRefl),
          _color(color) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return _color; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        static Callable impl = [](Float3 wo, Float3 wi) {
            Float Fo = schlick_weight(abs_cos_theta(wo));
            Float Fi = schlick_weight(abs_cos_theta(wi));
            return InvPi * (1 - 0.5f * Fo) * (1 - 0.5f * Fi);
        };
        return _color * impl(wi, wi);
    }
};

class FakeSS : public BxDF {
private:
    Float3 _color;
    Float _roughness;

public:
    FakeSS() = default;
    explicit FakeSS(Float3 color, Float r)
        : BxDF(BxDFFlag::DiffRefl),
          _color(color),
          _roughness(r) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return _color; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        static Callable impl = [](Float3 wo, Float3 wi, Float roughness) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float cos_theta_d = dot(wi, wh);
            Float Fss90 = sqr(cos_theta_d) * roughness;
            Float Fo = schlick_weight(abs_cos_theta(wo));
            Float Fi = schlick_weight(abs_cos_theta(wi));
            Float Fss = lerp(Fo, 1.f, Fss90) * lerp(Fi, 1.f, Fss90);
            Float ss = 1.25f * (Fss * (1 / (abs_cos_theta(wo) + abs_cos_theta(wi)) - .5f) + .5f);
            Float ret = InvPi * ss;
            return select(valid, ret, 0.f);
        };
        return _color * impl(wo, wi, _roughness);
    }
};

class Retro : public BxDF {
private:
    Float3 _color;
    Float _roughness;

public:
    Retro() = default;
    explicit Retro(Float3 color, Float r)
        : BxDF(BxDFFlag::DiffRefl),
          _color(color),
          _roughness(r) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return _color; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        static Callable impl = [](Float3 wo, Float3 wi, Float roughness) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float cos_theta_d = dot(wi, wh);

            Float Fo = schlick_weight(abs_cos_theta(wo));
            Float Fi = schlick_weight(abs_cos_theta(wi));
            Float Rr = 2 * roughness * sqr(cos_theta_d);

            Float ret = InvPi * Rr * (Fo + Fi + Fo * Fi * (Rr - 1));
            return select(valid, ret, 0.f);
        };
        return _color * impl(wo, wi, _roughness);
    }
};

class Sheen : public BxDF {
private:
    Float3 _color;

public:
    Sheen() = default;
    explicit Sheen(Float3 kr)
        : BxDF(BxDFFlag::DiffRefl),
          _color(kr) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return _color; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        static Callable impl = [](Float3 wo, Float3 wi) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float cos_theta_d = dot(wi, wh);
            Float ret = schlick_weight(cos_theta_d);
            return select(valid, ret, 0.f);
        };
        return _color * impl(wo, wi);
    }
};

[[nodiscard]] Float GTR1(Float cos_theta, Float alpha) {
    static Callable impl = [](Float cos_theta, Float alpha) {
        Float alpha2 = sqr(alpha);
        return (alpha2 - 1) /
               (Pi * log(alpha2) * (1 + (alpha2 - 1) * sqr(cos_theta)));
    };
    return impl(cos_theta, alpha);
}

[[nodiscard]] Float smithG_GGX(Float cos_theta, Float alpha) {
    static Callable impl = [](Float cos_theta, Float alpha) {
        Float alpha2 = sqr(alpha);
        Float cos_theta_2 = sqr(cos_theta);
        return 1 / (cos_theta + sqrt(alpha2 + cos_theta_2 - alpha2 * cos_theta_2));
    };
    return impl(cos_theta, alpha);
}

class Clearcoat : public BxDF {
private:
    Float _weight;
    Float _alpha;

public:
    Clearcoat() = default;
    Clearcoat(Float weight, Float alpha)
        : BxDF(BxDFFlag::GlossyRefl),
          _weight(weight),
          _alpha(alpha) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return make_float3(_weight); }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        static Callable impl = [](Float3 wo, Float3 wi, Float weight, Float alpha) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float Dr = GTR1(abs_cos_theta(wh), alpha);
            Float Fr = fresnel_schlick((0.04f), dot(wo, wh));
            Float Gr = smithG_GGX(abs_cos_theta(wo), 0.25f) * smithG_GGX(abs_cos_theta(wi), 0.25f);
            Float ret = weight * Gr * Fr * Dr * 0.25f;
            return make_float3(select(valid, ret, 0.f));
        };
        return impl(wo, wi, _weight, _alpha);
    }
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        static Callable impl = [](Float3 wo, Float3 wi, Float alpha) {
            Float3 wh = wi + wo;
            Bool valid = !is_zero(wh);
            wh = normalize(wh);
            Float Dr = GTR1(abs_cos_theta(wh), alpha);
            Float ret = Dr * abs_cos_theta(wh) / (4 * dot(wo, wh));
            return select(valid, ret, 0.f);
        };
        return impl(wo, wi, _alpha);
    }
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override {
        static Callable impl = [](Float3 wo, Float2 u, Float alpha) {
            Float alpha2 = sqr(alpha);
            Float cos_theta = safe_sqrt((1 - pow(alpha2, 1 - u[0])) / (1 - alpha2));
            Float sin_theta = safe_sqrt(1 - sqr(cos_theta));
            Float phi = 2 * Pi * u[1];
            Float3 wh = spherical_direction(sin_theta, cos_theta, phi);
            wh = select(same_hemisphere(wo, wh), wh, -wh);
            return reflect(wo, wh);
        };
        Float3 wi = impl(wo, u, _alpha);
        return {wi, true};
    }
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override {
        auto [wi, valid] = sample_wi(wo, u, fresnel);
        BSDFSample ret;
        ret.eval = safe_evaluate(wo, wi, nullptr);
        ret.wi = wi;
        ret.flags = BxDFFlag::GlossyRefl;
        return ret;
    }

    [[nodiscard]] SP<BSDFSample> sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept override {
        Float2 u = sampler->next_2d();
        auto [wi, valid] = sample_wi(wo, u, fresnel);
        auto ret = make_shared<BSDFSample>();
        ret->eval = safe_evaluate(wo, wi, nullptr);
        ret->wi = wi;
        ret->flags = BxDFFlag::GlossyRefl;
        return ret;
    }
};

class FresnelDisney : public Fresnel {
private:
    Float3 R0;
    Float _metallic;
    Float _eta;

public:
    FresnelDisney(const Float3 &R0, Float metallic, Float eta)
        : R0(R0), _metallic(metallic), _eta(eta) {}
    void correct_eta(Float cos_theta) noexcept override {
        _eta = select(cos_theta > 0, _eta, rcp(_eta));
    }
    [[nodiscard]] Float3 evaluate(Float cos_theta) const noexcept override {
        return lerp(make_float3(_metallic),
                    make_float3(fresnel_dielectric(cos_theta, _eta)),
                    fresnel_schlick(R0, cos_theta));
    }
    [[nodiscard]] Float eta() const noexcept override { return _eta; }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelDisney>(R0, _metallic, _eta);
    }
};

}// namespace disney

class PrincipledBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel{};
    optional<Diffuse> _diffuse{};
    optional<Retro> _retro{};
    optional<Sheen> _sheen{};
    optional<FakeSS> _fake_ss{};
    optional<MicrofacetReflection> _spec_refl{};
    optional<Clearcoat> _clearcoat{};
    optional<MicrofacetTransmission> _spec_trans{};

    // sampling strategy
    static constexpr size_t max_sampling_strategy_num = 4u;
    array<Float, max_sampling_strategy_num> _sampling_weights;
    uint _diffuse_index{InvalidUI32};
    uint _spec_refl_index{InvalidUI32};
    uint _clearcoat_index{InvalidUI32};
    uint _spec_trans_index{InvalidUI32};
    uint _sampling_strategy_num{0u};

private:
    template<typename T, typename... Args>
    [[nodiscard]] Float3 lobe_f(const optional<T> &lobe, Args &&...args) const noexcept {
        if (lobe.has_value()) {
            return OC_FORWARD(lobe)->f(OC_FORWARD(args)...);
        }
        return make_float3(0.f);
    }

    template<typename T, typename... Args>
    [[nodiscard]] Float lobe_PDF(const optional<T> &lobe, Args &&...args) const noexcept {
        if (lobe.has_value()) {
            return OC_FORWARD(lobe)->PDF(OC_FORWARD(args)...);
        }
        return 0.f;
    }

    template<typename... Args>
    [[nodiscard]] Float3 f_diffuse(Args &&...args) const noexcept {
        return lobe_f(_diffuse, OC_FORWARD(args)...) + lobe_f(_retro, OC_FORWARD(args)...) +
               lobe_f(_sheen, OC_FORWARD(args)...) + lobe_f(_fake_ss, OC_FORWARD(args)...);
    }

    template<typename... Args>
    [[nodiscard]] Float PDF_diffuse(Args &&...args) const noexcept {
        return lobe_PDF(_diffuse, OC_FORWARD(args)...);
    }

public:
    PrincipledBSDF(const Interaction &si, const Texture *color_tex, const Texture *metallic_tex, const Texture *eta_tex,
                   const Texture *roughness_tex, const Texture *spec_tint_tex, const Texture *anisotropic_tex, const Texture *sheen_tex,
                   const Texture *sheen_tint_tex, const Texture *clearcoat_tex, const Texture *clearcoat_alpha_tex,
                   const Texture *spec_trans_tex, const Texture *flatness_tex, const Texture *diff_trans_tex)
        : BSDF(si) {
        Float3 color = Texture::eval(color_tex, si).xyz();
        Float color_lum = luminance(color);
        Float metallic = Texture::eval(metallic_tex, si).x;
        Float spec_trans = Texture::eval(spec_trans_tex, si).x;
        Float diffuse_weight = (1.f - metallic) * (1 - spec_trans);
        Float flatness = Texture::eval(flatness_tex, si).x;
        Float roughness = Texture::eval(roughness_tex, si).x;
        Float tint_weight = select(color_lum > 0.f, 1.f / color_lum, 1.f);
        Float3 tint = clamp(color * tint_weight, make_float3(0.f), make_float3(1.f));
        Float tint_lum = color_lum * tint_weight;

        Float Cdiff_weight = diffuse_weight * (1.f - flatness);
        Float3 Cdiff = color * Cdiff_weight;

        bool has_diffuse = false;

        if (Texture::nonzero(color_tex)) {
            _diffuse = Diffuse(Cdiff);
            _retro = Retro(Cdiff, roughness);
            has_diffuse = true;
        }

        if (Texture::nonzero(flatness_tex)) {
            Float Css_weight = diffuse_weight * flatness;
            Float3 Css = Css_weight * color;
            _fake_ss = FakeSS(Css, roughness);
            has_diffuse = true;
        }

        if (Texture::nonzero(sheen_tex)) {
            Float sheen = Texture::eval(sheen_tex, si).x;
            Float sheen_tint = Texture::eval(sheen_tint_tex, si).x;
            Float Csheen_weight = diffuse_weight * sheen;
            Float3 Csheen = Csheen_weight * lerp(make_float3(sheen_tint), make_float3(1.f), tint);
            _sheen = Sheen(Csheen);
            has_diffuse = true;
        }

        if (has_diffuse) {
            _diffuse_index = _sampling_strategy_num++;
            _sampling_weights[_diffuse_index] = saturate(diffuse_weight * color_lum);
        }

        Float spec_tint = Texture::eval(spec_tint_tex, si).x;
        Float eta = Texture::eval(eta_tex, si).x;
        Float SchlickR0 = schlick_R0_from_eta(eta);
        Float3 Cspec0 = lerp(make_float3(metallic), lerp(make_float3(spec_tint), make_float3(1.f), tint) * SchlickR0, color);

        _fresnel = make_shared<FresnelDisney>(Cspec0, metallic, eta);
        Float anisotropic = Texture::eval(anisotropic_tex, si).x;
        Float aspect = sqrt(1 - anisotropic * 0.9f);
        Float2 alpha = make_float2(max(0.001f, sqr(roughness) / aspect),
                                   max(0.001f, sqr(roughness) * aspect));
        auto microfacet = make_shared<Microfacet<D>>(alpha, MicrofacetType::Disney);
        _spec_refl = MicrofacetReflection(make_float3(1.f), microfacet);
        Float Cspec0_lum = lerp(metallic, lerp(spec_tint, 1.f, tint_lum) * SchlickR0, color_lum);
        _spec_refl_index = _sampling_strategy_num++;
        _sampling_weights[_spec_refl_index] = saturate(Cspec0_lum);

        if (Texture::nonzero(clearcoat_tex)) {
            Float cc = Texture::eval(clearcoat_tex, si).x;
            Float cc_alpha = lerp(Texture::eval(clearcoat_alpha_tex, si).x, 0.001f, 1.f);
            _clearcoat = Clearcoat(cc, cc_alpha);
            _clearcoat_index = _sampling_strategy_num++;
            _sampling_weights[_clearcoat_index] = saturate(cc * fresnel_schlick(0.04f, 1.f));
        }

        if (Texture::nonzero(spec_trans_tex)) {
            Float Cst_weight = (1.f - metallic) * spec_trans;
            Float3 Cst = Cst_weight * sqrt(color);
            _spec_trans = MicrofacetTransmission(Cst, microfacet);
            Float Cst_lum = Cst_weight * sqrt(color_lum);
            _spec_trans_index = _sampling_strategy_num++;
            _sampling_weights[_spec_trans_index] = saturate(Cst_lum);
        }

        Float sum_weights = 0.f;
        for (uint i = 0u; i < _sampling_strategy_num; i++) {
            sum_weights += _sampling_weights[i];
        }
        Float inv_sum_weights = select(sum_weights == 0.f, 0.f, 1.f / sum_weights);
        for (uint i = 0u; i < _sampling_strategy_num; i++) {
            _sampling_weights[i] *= inv_sum_weights;
        }
    }
    [[nodiscard]] Float3 albedo() const noexcept override { return _diffuse->albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        ScatterEval ret;
        Float3 f = make_float3(0.f);
        Float pdf = 0.f;
        auto fresnel = _fresnel->clone();
        Float cos_theta_o = cos_theta(wo);
        fresnel->correct_eta(cos_theta_o);
        $if(same_hemisphere(wo, wi)) {
            if (_diffuse_index != InvalidUI32) {
                f = f_diffuse(wo, wi, fresnel);
                pdf = _sampling_weights[_diffuse_index] * PDF_diffuse(wo, wi, fresnel);
            }

            f += _spec_refl->f(wo, wi, fresnel);
            pdf += _sampling_weights[_spec_refl_index] * _spec_refl->PDF(wo, wi, fresnel);

            if (_clearcoat.has_value()) {
                f += _clearcoat->f(wo, wi, fresnel);
                pdf += _sampling_weights[_clearcoat_index] * _clearcoat->PDF(wo, wi, fresnel);
            }
        }
        $else {
            if (_spec_trans.has_value()) {
                f = _spec_trans->f(wo, wi, fresnel);
                pdf = _sampling_weights[_spec_trans_index] * _spec_trans->PDF(wo, wi, fresnel);
            }
        };
        ret.f = f;
        ret.pdf = pdf;
        return ret;
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept override {
        BSDFSample ret;

        Uint sampling_strategy = 0u;
        Float sum_weights = 0.f;
        for (uint i = 0; i < _sampling_strategy_num; ++i) {
            sampling_strategy = select(uc > sum_weights, i, sampling_strategy);
            sum_weights += _sampling_weights[i];
        }
        Float3 f;
        Float pdf;
        auto fresnel = _fresnel->clone();
        Float cos_theta_o = cos_theta(wo);
        fresnel->correct_eta(cos_theta_o);
        SampledDirection sampled_direction;
        $switch(sampling_strategy) {
            if (_diffuse.has_value()) {
                $case(_diffuse_index) {
                    sampled_direction = _diffuse->sample_wi(wo, u, fresnel);
                    $break;
                };
            }
            $case(_spec_refl_index) {
                sampled_direction = _spec_refl->sample_wi(wo, u, fresnel);
                $break;
            };
            if (_clearcoat.has_value()) {
                $case(_clearcoat_index) {
                    sampled_direction = _clearcoat->sample_wi(wo, u, fresnel);
                    $break;
                };
            }
            if (_spec_trans.has_value()) {
                $case(_spec_trans_index) {
                    sampled_direction = _spec_trans->sample_wi(wo, u, fresnel);
                    $break;
                };
            }
            $default {
                unreachable();
                $break;
            };
        };
        ret.eval = evaluate_local(wo, sampled_direction.wi, flag);
        ret.wi = sampled_direction.wi;
        ret.eval.pdf = select(sampled_direction.valid, ret.eval.pdf, 0.f);
        return ret;
    }

    [[nodiscard]] SP<ScatterSample> sample_local(Float3 wo, Uchar flag, Sampler *sampler) const noexcept override {
        auto ret = make_shared<BSDFSample>();
        Float uc = sampler->next_1d();
        Float2 u = sampler->next_2d();

        Uint sampling_strategy = 0u;
        Float sum_weights = 0.f;
        for (uint i = 0; i < _sampling_strategy_num; ++i) {
            sampling_strategy = select(uc > sum_weights, i, sampling_strategy);
            sum_weights += _sampling_weights[i];
        }
        Float3 f;
        Float pdf;
        auto fresnel = _fresnel->clone();
        Float cos_theta_o = cos_theta(wo);
        fresnel->correct_eta(cos_theta_o);
        SampledDirection sampled_direction;
        $switch(sampling_strategy) {
            if (_diffuse.has_value()) {
                $case(_diffuse_index) {
                    sampled_direction = _diffuse->sample_wi(wo, u, fresnel);
                    $break;
                };
            }
            $case(_spec_refl_index) {
                sampled_direction = _spec_refl->sample_wi(wo, u, fresnel);
                $break;
            };
            if (_clearcoat.has_value()) {
                $case(_clearcoat_index) {
                    sampled_direction = _clearcoat->sample_wi(wo, u, fresnel);
                    $break;
                };
            }
            if (_spec_trans.has_value()) {
                $case(_spec_trans_index) {
                    sampled_direction = _spec_trans->sample_wi(wo, u, fresnel);
                    $break;
                };
            }
            $default {
                unreachable();
                $break;
            };
        };
        ret->eval = evaluate_local(wo, sampled_direction.wi, flag);
        ret->wi = sampled_direction.wi;
        ret->eval.pdf = select(sampled_direction.valid, ret->eval.pdf, 0.f);
        return ret;
    }
};

class DisneyMaterial : public Material {
private:
    const Texture *_color{};
    const Texture *_metallic{};
    const Texture *_eta{};
    const Texture *_roughness{};
    const Texture *_spec_tint{};
    const Texture *_anisotropic{};
    const Texture *_sheen{};
    const Texture *_sheen_tint{};
    const Texture *_clearcoat{};
    const Texture *_clearcoat_alpha{};
    const Texture *_spec_trans{};
    const Texture *_flatness{};
    const Texture *_diff_trans{};
    bool _thin;

public:
    explicit DisneyMaterial(const MaterialDesc &desc)
        : Material(desc), _color(desc.scene->load<Texture>(desc.color)),
          _metallic(desc.scene->load<Texture>(desc.metallic)),
          _eta(desc.scene->load<Texture>(desc.ior)),
          _roughness(desc.scene->load<Texture>(desc.roughness)),
          _spec_tint(desc.scene->load<Texture>(desc.spec_tint)),
          _anisotropic(desc.scene->load<Texture>(desc.anisotropic)),
          _sheen(desc.scene->load<Texture>(desc.sheen)),
          _sheen_tint(desc.scene->load<Texture>(desc.sheen_tint)),
          _clearcoat(desc.scene->load<Texture>(desc.clearcoat)),
          _clearcoat_alpha(desc.scene->load<Texture>(desc.clearcoat_alpha)),
          _spec_trans(desc.scene->load<Texture>(desc.spec_trans)),
          _flatness(desc.scene->load<Texture>(desc.flatness)),
          _diff_trans(desc.scene->load<Texture>(desc.diff_trans)) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si) const noexcept override {
        return make_unique<PrincipledBSDF>(si, _color, _metallic, _eta, _roughness, _spec_tint, _anisotropic,
                                           _sheen, _sheen_tint, _clearcoat, _clearcoat_alpha,
                                           _spec_trans, _flatness, _diff_trans);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::DisneyMaterial)