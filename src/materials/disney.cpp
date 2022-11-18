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
        Float Fo = schlick_weight(abs_cos_theta(wo));
        Float Fi = schlick_weight(abs_cos_theta(wi));
        return _color * InvPi * (1 - 0.5f * Fo) * (1 - 0.5f * Fi);
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
        Float3 wh = wi + wo;
        Bool valid = !is_zero(wh);
        wh = normalize(wh);
        Float cos_theta_d = dot(wi, wh);
        Float Fss90 = sqr(cos_theta_d) * _roughness;
        Float Fo = schlick_weight(abs_cos_theta(wo));
        Float Fi = schlick_weight(abs_cos_theta(wi));
        Float Fss = lerp(Fo, 1.f, Fss90) * lerp(Fi, 1.f, Fss90);
        Float ss = 1.25f * (Fss * (1 / (abs_cos_theta(wo) + abs_cos_theta(wi)) - .5f) + .5f);
        Float3 ret = _color * InvPi * ss;
        return select(valid, ret, make_float3(0));
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
        Float3 wh = wi + wo;
        Bool valid = !is_zero(wh);
        wh = normalize(wh);
        Float cos_theta_d = dot(wi, wh);

        Float Fo = schlick_weight(abs_cos_theta(wo));
        Float Fi = schlick_weight(abs_cos_theta(wi));
        Float Rr = 2 * _roughness * sqr(cos_theta_d);

        Float3 ret = _color * InvPi * Rr * (Fo + Fi + Fo * Fi * (Rr - 1));
        return select(valid, ret, make_float3(0));
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
        Float3 wh = wi + wo;
        Bool valid = !is_zero(wh);
        wh = normalize(wh);
        Float cos_theta_d = dot(wi, wh);
        Float3 ret = _color * schlick_weight(cos_theta_d);
        return select(valid, ret, make_float3(0));
    }
};

[[nodiscard]] Float GTR1(Float cos_theta, Float alpha) {
    Float alpha2 = sqr(alpha);
    return (alpha2 - 1) /
           (Pi * log(alpha2) * (1 + (alpha2 - 1) * sqr(cos_theta)));
}

[[nodiscard]] Float smithG_GGX(Float cos_theta, Float alpha) {
    Float alpha2 = sqr(alpha);
    Float cos_theta_2 = sqr(cos_theta);
    return 1 / (cos_theta + sqrt(alpha2 + cos_theta_2 - alpha2 * cos_theta_2));
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
        Float3 wh = wi + wo;
        Bool valid = !is_zero(wh);
        wh = normalize(wh);
        Float Dr = GTR1(abs_cos_theta(wh), _alpha);
        Float Fr = fresnel_schlick((0.04f), dot(wo, wh));
        Float Gr = smithG_GGX(abs_cos_theta(wo), 0.25f) * smithG_GGX(abs_cos_theta(wi), 0.25f);
        Float ret = _weight * Gr * Fr * Dr * 0.25f;
        return make_float3(select(valid, ret, 0.f));
    }
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        Float3 wh = wi + wo;
        Bool valid = !is_zero(wh);
        wh = normalize(wh);
        Float Dr = GTR1(abs_cos_theta(wh), _alpha);
        Float ret = Dr * abs_cos_theta(wh) / (4 * dot(wo, wh));
        return select(valid, ret, 0.f);
    }
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override {
        Float alpha2 = sqr(_alpha);
        Float cos_theta = safe_sqrt((1 - pow(alpha2, 1 - u[0])) / (1 - alpha2));
        Float sin_theta = safe_sqrt(1 - sqr(cos_theta));
        Float phi = 2 * Pi * u[1];
        Float3 wh = spherical_direction(sin_theta, cos_theta, phi);
        wh = select(same_hemisphere(wo, wh), wh, -wh);
        Float3 wi = reflect(wo, wh);
        BSDFSample ret;
        ret.eval = safe_evaluate(wo, wi, nullptr);
        ret.wi = wi;
        ret.flags = BxDFFlag::GlossyRefl;
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
    Diffuse _diffuse;
//    FakeSS _fake_ss;
    Retro _retro;
    Sheen _sheen;
    MicrofacetReflection _spec_refl;
    Clearcoat _clearcoat;
    MicrofacetTransmission _spec_trans;

public:
    PrincipledBSDF(const SurfaceInteraction &si, Float3 color, Float metallic, Float eta, Float roughness,
                   Float spec_tint, Float anisotropic, Float sheen, Float sheen_tint, Float clearcoat,
                   Float clearcoat_alpha, Float spec_trans, Float flatness, Float diff_trans) : BSDF(si) {
        Float diffuse_weight = (1 - metallic) * (1 - spec_trans);
        Float lum = luminance(color);
        Float3 color_tint = select(lum > 0, color / lum, make_float3(1.f));
        Float3 color_sheen_tint = select(sheen > 0,
                                         lerp(make_float3(sheen_tint), make_float3(1.f), color_tint),
                                         make_float3(0.f));
        Float3 R0 = lerp(Float3(metallic),
                         schlick_R0_from_eta(eta) * lerp(Float3(spec_tint), Float3(1.f), color_sheen_tint),
                         color);
        Float dt = diff_trans * 0.5f;
        Float aspect = safe_sqrt(1 - anisotropic * 0.9f);

        _diffuse = Diffuse(diffuse_weight * color);
//        _fake_ss = FakeSS(diffuse_weight * flatness * (1 - dt) * color, roughness);
        _retro = Retro(diffuse_weight * color, roughness);
        _sheen = Sheen(diffuse_weight * sheen * color_sheen_tint);

        Float ax = max(0.001f, sqr(roughness) / aspect);
        Float ay = max(0.001f, sqr(roughness) * aspect);

        SP<Microfacet<D>> microfacet = make_shared<Microfacet<D>>(ax, ay, MicrofacetType::Disney);
        _spec_refl = MicrofacetReflection(make_float3(1.f), microfacet);
        _clearcoat = Clearcoat(clearcoat, clearcoat_alpha);
        _spec_trans = MicrofacetTransmission(spec_trans * sqrt(color), microfacet);
    }
    [[nodiscard]] Float3 albedo() const noexcept override { return _diffuse.albedo(); }
    [[nodiscard]] BSDFEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        BSDFEval ret;
        return ret;
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept override {
        BSDFSample ret;
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

    [[nodiscard]] UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept override {
        Float3 color = eval_tex(_color, si, 0.f).xyz();
        Float metallic = eval_tex(_metallic, si, 1.5f).x;
        Float eta = eval_tex(_eta, si, 1.5f).x;
        Float roughness = eval_tex(_roughness, si, 0.5f).x;
        Float spec_tint = eval_tex(_spec_tint, si, 0.f).x;
        Float anisotropic = eval_tex(_anisotropic, si, 0.f).x;
        Float sheen = eval_tex(_sheen, si, 0.f).x;
        Float sheen_tint = eval_tex(_sheen_tint, si, 0.f).x;
        Float clearcoat = eval_tex(_clearcoat, si, 0.f).x;
        Float clearcoat_alpha = lerp(eval_tex(_clearcoat_alpha, si, 0.f).x, 0.001f, 1.f);
        Float spec_trans = eval_tex(_spec_trans, si, 0.f).x;
        Float flatness = eval_tex(_flatness, si, 0.f).x;
        Float diff_trans = eval_tex(_diff_trans, si, 0.f).x;
        return make_unique<PrincipledBSDF>(si, color, metallic, eta, roughness, spec_tint, anisotropic,
                                           sheen, sheen_tint, clearcoat, clearcoat_alpha,
                                           spec_trans, flatness, diff_trans);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::DisneyMaterial)