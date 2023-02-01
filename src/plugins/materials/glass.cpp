//
// Created by Zero on 05/10/2022.
//

#include "base/scattering/material.h"
#include "base/texture.h"
#include "base/mgr/scene.h"

namespace vision {

class IORCurve {
public:
    [[nodiscard]] virtual Float eta(const Float &lambda) const noexcept = 0;
    [[nodiscard]] virtual SampledSpectrum eta(const SampledWavelengths &swl) const noexcept = 0;
};

#define VS_IOR_CURVE_COMMON                                                                    \
    [[nodiscard]] Float eta(const Float &lambda) const noexcept override {                     \
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

class GlassMaterial : public Material {
private:
    const Texture *_color{};
    const Texture *_ior{};
    const Texture *_roughness{};
    bool _remapping_roughness{false};
    IORCurve *_ior_curve{nullptr};

public:
    explicit GlassMaterial(const MaterialDesc &desc)
        : Material(desc),
          _color(desc.scene->load<Texture>(desc.color)),
          _ior(desc.scene->load<Texture>(desc.ior)),
          _roughness(desc.scene->load<Texture>(desc.roughness)),
          _remapping_roughness(desc.remapping_roughness),
          _ior_curve(ior_curve(desc.material_name)) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum color = Texture::eval_albedo_spectrum(_color, si, swl).sample;
        Float ior;
        if (_ior_curve) {
            ior = _ior_curve->eta(swl.lambda(0u));
        } else {
            ior = Texture::eval(_ior, si, 1.5f).x;
        }
        Float2 alpha = Texture::eval(_roughness, si, 1.f).xy();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelDielectric>(SampledSpectrum{swl.dimension(), ior},
                                                      swl, render_pipeline());
        MicrofacetReflection refl(SampledSpectrum(swl.dimension(), 1.f), swl, microfacet);
        MicrofacetTransmission trans(color, swl, microfacet);
        return make_unique<DielectricBSDF>(si, fresnel, move(refl), move(trans));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GlassMaterial)