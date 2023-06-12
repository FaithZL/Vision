//
// Created by Zero on 09/09/2022.
//

#include "base/light.h"
#include "base/mgr/pipeline.h"
#include "base/shader_graph/shader_node.h"
#include "math/warp.h"

namespace vision {

class AreaLight : public Light {
private:
    using _serial_ty = Light;
    Serial<uint> _inst_idx{InvalidUI32};
    Serial<uint> _two_sided{0u};
    Warper *_warper{nullptr};

public:
    explicit AreaLight(const LightDesc &desc)
        : Light(desc, LightType::Area),
          _two_sided{desc["two_sided"].as_bool(false)},
          _inst_idx(desc["inst_id"].as_uint()) {
    }
    OC_SERIALIZABLE_FUNC(_inst_idx, _two_sided, *_warper)
    [[nodiscard]] Float PMF(const Uint &prim_id) const noexcept override {
        return _warper->PMF(prim_id);
    }

    [[nodiscard]] bool two_sided() const noexcept {
        return _two_sided.hv();
    }

    [[nodiscard]] float surface_area() const noexcept {
        float ret = 0.f;
        vector<float> weights = shape()->surface_area();
        for (float weight : weights) {
            ret += weight;
        }
        return ret;
    }

    [[nodiscard]] Shape *shape() const noexcept {
        return _scene->get_shape(_inst_idx.hv());
    }

    [[nodiscard]] float3 power() const noexcept override {
        return (two_sided() ? 2.f : 1.f) * average() * surface_area() * Pi;
    }

    [[nodiscard]] SampledSpectrum L(const LightEvalContext &p_light, const Float3 &w,
                                    const SampledWavelengths &swl) const {
        SampledSpectrum radiance = _color.eval_illumination_spectrum(p_light.uv, swl).sample * scale();
        return radiance * select(dot(w, p_light.ng) > 0 || (*_two_sided), 1.f, 0.f);
    }

    [[nodiscard]] SampledSpectrum Li(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        return L(p_light, p_ref.pos - p_light.pos, swl);
    }

    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        Float ret = PDF_dir(p_light.PDF_pos, p_light.ng, p_ref.pos - p_light.pos);
        return select(ocarina::isinf(ret), 0.f, ret);
    }

    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override {
        LightSample ret{swl.dimension()};
        auto rp = _scene->render_pipeline();
        Float pmf;
        Float u_remapped;
        Uint prim_id = _warper->sample_discrete(u.x, &pmf, &u_remapped);
        u.x = u_remapped;
        Float2 bary = square_to_triangle(u);
        LightEvalContext p_light = rp->compute_light_eval_context(*_inst_idx, prim_id, bary);
        p_light.PDF_pos *= pmf;
        ret.eval = evaluate(p_ref, p_light, swl);
        ret.p_light = p_light.robust_pos(p_ref.pos - p_light.pos);
        return ret;
    }

    void prepare() noexcept override {
        _warper = _scene->load_warper();
        vector<float> weights = shape()->surface_area();
        _warper->build(std::move(weights));
        _warper->prepare();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AreaLight)