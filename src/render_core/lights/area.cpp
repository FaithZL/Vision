//
// Created by Zero on 09/09/2022.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"
#include "base/shader_graph/shader_node.h"

namespace vision {

//    "type": "area",
//    "param": {
//        "color" : {
//            "channels" : "xyz",
//            "node" : [17,12,4]
//        },
//        "width" : 1,
//        "height" : 1,
//        "two_sided" : false,
//        "scale" : 1,
//        "o2w" : {
//            "type" : "matrix4x4",
//            "param" : {
//                "matrix4x4" : [
//                    [1,0,0,0],
//                    [0,1,0,0],
//                    [0,0,1,0],
//                    [0,0,0,1]
//                ]
//            }
//        }
//    }
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
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    OC_SERIALIZABLE_FUNC(IAreaLight, _two_sided, *_warper)

    void init_geometry(const LightDesc &desc) {
        ShapeDesc sd;
        sd.sub_type = "quad";
        sd.set_value("width", desc["width"].as_float(1));
        sd.set_value("height", desc["height"].as_float(1));
        sd.o2w = desc.o2w;
        SP<ShapeGroup> shape = Global::node_mgr().load<ShapeGroup>(sd);
        scene().groups().push_back(shape);
        _inst_idx = scene().instances().size();
        shape->for_each([&](ShapeInstance &instance, uint i) {
            instance.set_material(scene().obtain_black_body());
            scene().instances().push_back(instance);
            set_instance(&instance);
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
        vector<float> weights = _instance->surface_areas();
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

    [[nodiscard]] SampledSpectrum Le(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        return L(p_light, p_ref.pos - p_light.pos, swl);
    }

    [[nodiscard]] Float PDF_point(const vision::LightSampleContext &p_ref,
                                  const vision::LightEvalContext &p_light) const noexcept override {
        return p_light.PDF_pos;
    }

    [[nodiscard]] SampledSpectrum Li(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        return Le(p_ref, p_light, swl) * G(p_ref, p_light);
    }

    [[nodiscard]] Float PDF_wi(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        Float ret = vision::PDF_wi(p_light.PDF_pos, p_light.ng, p_ref.pos - p_light.pos);
        return select(ocarina::isinf(ret), 0.f, ret);
    }

    [[nodiscard]] LightEvalContext sample_surface(Float2 u) const noexcept {
        Float pmf;
        Uint prim_id = sample_primitive(addressof(u), addressof(pmf));
        return sample_surface(u, prim_id, pmf);
    }

    [[nodiscard]] LightEvalContext sample_surface(Float2 u, Uint prim_id, Float pmf) const noexcept {
        Float2 bary = square_to_triangle(u);
        auto rp = scene().pipeline();
        LightEvalContext p_light = rp->compute_light_eval_context(*_inst_idx, prim_id, bary);
        p_light.PDF_pos *= pmf;
        return p_light;
    }

    [[nodiscard]] Uint sample_primitive(ocarina::Float2 *u, Float *pmf) const noexcept override {
        Uint prim_id = _warper->sample_discrete(u->x, pmf,
                                                addressof(u->x));
        return prim_id;
    }

    [[nodiscard]] LightSurfacePoint sample_point(Float2 u) const noexcept override {
        Uint prim_id = _warper->sample_discrete(u.x, nullptr, addressof(u.x));
        LightSurfacePoint lsp;
        lsp.prim_id = prim_id;
        lsp.uv = square_to_triangle(u);
        return lsp;
    }

    [[nodiscard]] LightSample sample_wi(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override {
        LightSample ret{swl.dimension()};
        LightEvalContext p_light = sample_surface(u);
        ret.eval = evaluate_wi(p_ref, p_light, swl);
        ret.p_light = p_light.robust_pos(p_ref.pos - p_light.pos);
        return ret;
    }

    [[nodiscard]] LightSample evaluate(const LightSampleContext &p_ref, const LightSurfacePoint &lsp,
                                       const SampledWavelengths &swl) const noexcept override {
        LightSample ret{swl.dimension()};
        Float pmf = _warper->PDF(lsp.prim_id);
        auto rp = scene().pipeline();
        LightEvalContext p_light = rp->compute_light_eval_context(*_inst_idx, lsp.prim_id, lsp.uv);
        ret.eval = evaluate_point(p_ref, p_light, swl);
        ret.p_light = p_light.robust_pos(p_ref.pos - p_light.pos);
        return ret;
    }

    [[nodiscard]] LightSample sample_point(const LightSampleContext &p_ref, Float2 u,
                                           const SampledWavelengths &swl) const noexcept override {
        LightSample ret{swl.dimension()};
        LightEvalContext p_light = sample_surface(u);
        ret.eval = evaluate_point(p_ref, p_light, swl);
        ret.p_light = p_light.robust_pos(p_ref.pos - p_light.pos);
        return ret;
    }

    void prepare() noexcept override {
        _warper = scene().load_warper();
        vector<float> weights = instance()->surface_areas();
        _warper->build(std::move(weights));
        _warper->prepare();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AreaLight)