//
// Created by Zero on 09/09/2022.
//

#include "base/light.h"
#include "core/render_pipeline.h"
#include "base/texture.h"
#include "math/warp.h"

namespace vision {

class AreaLight : public Light {
private:
    uint _inst_idx{InvalidUI32};
    bool _two_sided{false};
    Distribution *_distribution{nullptr};
    Texture *_radiance{nullptr};

public:
    explicit AreaLight(const LightDesc &desc)
        : Light(desc, LightType::Area),
          _two_sided{desc.two_sided}, _inst_idx(desc.inst_id) {
        _radiance = desc.scene->load<Texture>(desc.radiance);
    }

    [[nodiscard]] Float PMF(const Uint &prim_id) const noexcept override {
        return _distribution->PMF(prim_id);
    }

    [[nodiscard]] Float3 L(const LightEvalContext &p_light, const Float3 &w) const {
        if (_two_sided) {
            return _radiance->eval(p_light.uv).xyz();
        }
        return select(dot(w, p_light.ng) > 0, _radiance->eval(p_light.uv).xyz(), make_float3(0.f));
    }

    [[nodiscard]] Float3 Li(const LightSampleContext &p_ref,
                            const LightEvalContext &p_light) const noexcept override {
        return L(p_light, p_ref.pos - p_light.pos);
    }

    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        Float ret = PDF_dir(p_light.PDF_pos, p_light.ng, p_ref.pos - p_light.pos);
        return select(isinf(ret), 0.f, ret);
    }

    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u) const noexcept override {
        LightSample ret;
        DeviceData &data = _scene->render_pipeline()->device_data();
        auto [prim_id, pmf, u_remapping] = _distribution->sample_discrete(u.x);
        u.x = u_remapping;
        Float2 bary = square_to_triangle(u);
        LightEvalContext p_light = data.compute_light_eval_context(_inst_idx, prim_id, bary);
        p_light.PDF_pos *= pmf;
        Float3 wi_un = p_light.pos - p_ref.pos;
        ret.eval = evaluate(p_ref, p_light);
        ret.wi = normalize(wi_un);
        ret.distance = length(wi_un);
        return ret;
    }

    void prepare(RenderPipeline *rp) noexcept override {
        _distribution = rp->scene().load_distribution();
        Shape *shape = rp->scene().get_shape(_inst_idx);
        vector<float> weights = shape->surface_area();
        _distribution->build(std::move(weights));
        _distribution->prepare(rp);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AreaLight)