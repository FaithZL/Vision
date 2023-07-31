//
// Created by Zero on 09/09/2022.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"
#include "base/shader_graph/shader_node.h"
#include "math/warp.h"

namespace vision {

class AreaLight : public IAreaLight {
private:
    Serial<uint> _two_sided{0u};
    SP<Warper> _warper{nullptr};

public:
    explicit AreaLight(const LightDesc &desc)
        : IAreaLight(desc),
          _two_sided{desc["two_sided"].as_bool(false)} {
        if (_inst_idx.hv() == InvalidUI32) {
            init_geometry(desc);
        }
    }

    OC_SERIALIZABLE_FUNC(IAreaLight, _two_sided, *_warper)

    void init_geometry(const LightDesc &desc) {
        ShapeDesc sd;
        sd.sub_type = "quad";
        sd.set_value("width", desc["width"].as_float(1));
        sd.set_value("height", desc["height"].as_float(1));
        sd.o2w = desc.o2w;
        SP<Shape> shape = Global::node_mgr().load<Shape>(sd);
        scene().shapes().push_back(shape);
        _inst_idx = scene().meshes().size();
        shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
            mesh._material.object = scene().obtain_black_body();
            scene().meshes().push_back(&mesh);
            set_mesh(&mesh);
        });
    }

    [[nodiscard]] Float PMF(const Uint &prim_id) const noexcept override {
        return _warper->PMF(prim_id);
    }

    [[nodiscard]] bool two_sided() const noexcept {
        return _two_sided.hv();
    }

    [[nodiscard]] float surface_area() const noexcept {
        float ret = 0.f;
        vector<float> weights = mesh()->surface_areas();
        for (float weight : weights) {
            ret += weight;
        }
        return ret;
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
        auto rp = scene().pipeline();
        Float pmf;
        Float u_remapped;
        Uint prim_id = _warper->sample_discrete(u.x, addressof(pmf),
                                                addressof(u_remapped));
        u.x = u_remapped;
        Float2 bary = square_to_triangle(u);
        LightEvalContext p_light = rp->compute_light_eval_context(*_inst_idx, prim_id, bary);
        p_light.PDF_pos *= pmf;
        ret.eval = evaluate(p_ref, p_light, swl);
        ret.p_light = p_light.robust_pos(p_ref.pos - p_light.pos);
        return ret;
    }

    void prepare() noexcept override {
        _warper = scene().load_warper();
        vector<float> weights = mesh()->surface_areas();
        _warper->build(std::move(weights));
        _warper->prepare();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AreaLight)