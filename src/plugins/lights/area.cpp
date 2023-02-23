//
// Created by Zero on 09/09/2022.
//

#include "base/light.h"
#include "base/mgr/render_pipeline.h"
#include "base/texture.h"
#include "math/warp.h"

namespace vision {

class AreaLight : public Light {
private:
    uint _inst_idx{InvalidUI32};
    bool _two_sided{false};
    Warper *_warper{nullptr};
    Texture *_radiance{nullptr};
    float _scale{1.f};

public:
    explicit AreaLight(const LightDesc &desc)
        : Light(desc, LightType::Area),
          _two_sided{desc.two_sided}, _inst_idx(desc.inst_id), _scale(desc.scale) {
        _radiance = desc.scene->load_texture(desc.texture_desc);
    }

    [[nodiscard]] Float PMF(const Uint &prim_id) const noexcept override {
        return _warper->PMF(prim_id);
    }

    [[nodiscard]] SampledSpectrum L(const LightEvalContext &p_light, const Float3 &w,
                                    const SampledWavelengths &swl) const {
        SampledSpectrum radiance = _radiance->eval_illumination_spectrum(p_light.uv, swl).sample * _scale;
        if (_two_sided) {
            return radiance;
        }
        return radiance * select(dot(w, p_light.ng) > 0, 1.f, 0.f);
    }

    [[nodiscard]] SampledSpectrum Li(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        return L(p_light, p_ref.pos - p_light.pos, swl);
    }

    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        Float ret = PDF_dir(p_light.PDF_pos, p_light.ng, p_ref.pos - p_light.pos);
        return select(isinf(ret), 0.f, ret);
    }

    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override {
        LightSample ret{swl.dimension()};
        auto rp = _scene->render_pipeline();
        auto [prim_id, pmf, u_remapping] = _warper->sample_discrete(u.x);
        u.x = u_remapping;
        Float2 bary = square_to_triangle(u);
        LightEvalContext p_light = rp->compute_light_eval_context(_inst_idx, prim_id, bary);
        p_light.PDF_pos *= pmf;
        ret.eval = evaluate(p_ref, p_light, swl);
        ret.p_light = p_light.robust_pos(p_ref.pos - p_light.pos);
        return ret;
    }

    void prepare() noexcept override {
        _warper = _scene->load_warper();
        Shape *shape = _scene->get_shape(_inst_idx);
        vector<float> weights = shape->surface_area();
        _warper->build(std::move(weights));
        _warper->prepare();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AreaLight)