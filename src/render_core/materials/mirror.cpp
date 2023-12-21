//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class MirrorBxDFSet : public BxDFSet {
private:
    deep_copy_shared_ptr<Fresnel> _fresnel;
    MicrofacetReflection _bxdf;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_fresnel->type_hash(), _bxdf.type_hash());
    }

public:
    MirrorBxDFSet(const SP<Fresnel> &fresnel, MicrofacetReflection bxdf)
        : _fresnel(fresnel), _bxdf(std::move(bxdf)) {}
    // clang-format off
    VS_MAKE_BxDFSet_ASSIGNMENT(MirrorBxDFSet)
    // clang-format on
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi, _fresnel->clone());
    }

    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _bxdf.sample(wo, sampler, _fresnel->clone());
    }

    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Uint flag,
                                             Sampler *sampler) const noexcept override {
        return _bxdf.sample_wi(wo, sampler->next_2d(), _fresnel->clone());
    }
};

class MirrorMaterial : public Material {
private:
    Slot _color{};
    Slot _roughness{};
    bool _remapping_roughness{true};

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<MirrorBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc), _color(scene().create_slot(desc.slot("color", make_float3(1.f), Albedo))),
          _roughness(scene().create_slot(desc.slot("roughness", make_float2(0.0001f)))),
          _remapping_roughness(desc["remapping_roughness"].as_bool(true)) {
        init_slot_cursor(&_color, 2);
    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }

protected:
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = _color.eval_albedo_spectrum(it, swl).sample;
        Float2 alpha = _roughness.evaluate(it, swl).as_vec2();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelNoOp>(swl, pipeline());
        MicrofacetReflection bxdf(kr, swl, microfacet);
        return make_unique<MirrorBxDFSet>(fresnel, ocarina::move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MirrorMaterial)