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

class ClearCoat : public BxDF {
private:
    Float _weight;
    Float _gloss;

public:
    ClearCoat(Float weight, Float gloss)
        : BxDF(BxDFFlag::GlossyRefl),
          _weight(weight),
          _gloss(gloss) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return make_float3(_weight); }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        Float3 wh = wi + wo;
        Bool valid = !is_zero(wh);
        wh = normalize(wh);
        Float Dr = GTR1(abs_cos_theta(wh), _gloss);
        Float Fr = fresnel_schlick((0.04f), dot(wo, wh));
        Float Gr = smithG_GGX(abs_cos_theta(wo), 0.25f) * smithG_GGX(abs_cos_theta(wi), 0.25f);
        Float ret = _weight * Gr * Fr * Dr * 0.25f;
        return make_float3(select(valid, ret, 0.f));
    }
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        Float3 wh = wi + wo;
        Bool valid = !is_zero(wh);
        wh = normalize(wh);
        Float Dr = GTR1(abs_cos_theta(wh), _gloss);
        Float ret = Dr * abs_cos_theta(wh) / (4 * dot(wo, wh));
        return select(valid, ret, 0.f);
    }
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override {
        Float alpha2 = sqr(_gloss);
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

}// namespace disney

class PrincipledBSDF : public BSDF {
};

class DisneyMaterial : public Material {
private:
    const Texture *_color{};
    const Texture *_metallic{};
    const Texture *_eta{};
    const Texture *_roughness{};
    const Texture *_specular_tint{};
    const Texture *_anisotropic{};
    const Texture *_sheen{};
    const Texture *_sheen_tint{};
    const Texture *_clearcoat{};
    const Texture *_clearcoat_gloss{};
    const Texture *_specular_trans{};
    const Texture *_flatness{};
    const Texture *_diffuse_trans{};
    bool _thin;
};

}// namespace vision