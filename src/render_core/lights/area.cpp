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
    EncodedData<uint> two_sided_{0u};
    SP<Warper> warper_{nullptr};

public:
    AreaLight() = default;
    explicit AreaLight(const LightDesc &desc)
        : IAreaLight(desc),
          two_sided_{desc["two_sided"].as_bool(false)} {
        if (inst_idx_.hv() == InvalidUI32) {
            init_geometry(desc);
        }
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    OC_ENCODABLE_FUNC(IAreaLight, two_sided_, *warper_)
    VS_HOTFIX_MAKE_RESTORE(IAreaLight, two_sided_, warper_)
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        IAreaLight::render_sub_UI(widgets);
        changed_ |= widgets->check_box("two sided",
                                       reinterpret_cast<bool *>(std::addressof(two_sided_.hv())));
    }
    void init_geometry(const LightDesc &desc) {
        ShapeDesc sd;
        sd.sub_type = "quad";
        sd.set_value("width", desc["width"].as_float(1));
        sd.set_value("height", desc["height"].as_float(1));
        sd.o2w = desc.o2w;
        SP<ShapeGroup> shape = Node::create_shared<ShapeGroup>(sd);
        scene().groups().push_back(shape);
        inst_idx_ = scene().instances().size();
        shape->for_each([&](const SP<ShapeInstance> &instance, uint i) {
            instance->set_material(TObject<Material>(scene().obtain_black_body()));
            scene().instances().push_back(instance);
        });
    }

    [[nodiscard]] Float PMF(const Uint &prim_id) const noexcept override {
        return warper_->PMF(prim_id);
    }

    [[nodiscard]] bool two_sided() const noexcept {
        return two_sided_.hv();
    }

    [[nodiscard]] float surface_area() const noexcept {
        float ret = 0.f;
        vector<float> weights = instance()->surface_areas();
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
        SampledSpectrum radiance = color_.eval_illumination_spectrum(p_light.uv, swl).sample * scale();
        return radiance * select(dot(w, p_light.ng) > 0 || (*two_sided_), 1.f, 0.f);
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
        return select(ocarina::isinf(ret) || ocarina::isnan(ret), 0.f, ret);
    }

    [[nodiscard]] LightEvalContext sample_surface(Float2 u) const noexcept {
        Float pmf;
        Uint prim_id = warper_->sample_discrete(u.x, addressof(pmf), addressof(u.x));
        return sample_surface(u, prim_id, pmf);
    }

    [[nodiscard]] LightEvalContext sample_surface(Float2 u, Uint prim_id, Float pmf) const noexcept {
        Float2 bary = square_to_triangle(u);
        auto rp = scene().pipeline();
        LightEvalContext p_light = rp->compute_light_eval_context(*inst_idx_, prim_id, bary);
        p_light.PDF_pos *= pmf;
        return p_light;
    }

    [[nodiscard]] LightSurfacePoint sample_only(Float2 u) const noexcept override {
        Uint prim_id = warper_->sample_discrete(u.x, nullptr, addressof(u.x));
        LightSurfacePoint lsp;
        lsp.prim_id = prim_id;
        lsp.bary = square_to_triangle(u);
        return lsp;
    }

    [[nodiscard]] LightSample sample_wi(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override {
        LightSample ret{swl.dimension()};
        LightEvalContext p_light = sample_surface(u);
        ret.eval = evaluate_wi(p_ref, p_light, swl, LightEvalMode::All);
        ret.p_light = p_light.robust_pos(p_ref.pos - p_light.pos);
        return ret;
    }

    [[nodiscard]] LightEvalContext compute_light_eval_context(const LightSampleContext &p_ref,
                                                              LightSurfacePoint lsp) const noexcept override {
        auto rp = scene().pipeline();
        return rp->compute_light_eval_context(*inst_idx_, lsp.prim_id, lsp.bary);
    }

    [[nodiscard]] LightSample evaluate_point(const LightSampleContext &p_ref, LightSurfacePoint lsp,
                                             const SampledWavelengths &swl, LightEvalMode mode) const noexcept override {
        LightSample ret{swl.dimension()};
        LightEvalContext p_light = compute_light_eval_context(p_ref, lsp);
        ret.eval = _evaluate_point(p_ref, p_light, swl, mode);
        if (match_PDF(mode)) {
            Float pmf = warper_->PMF(lsp.prim_id);
            ret.eval.pdf *= pmf;
        }
        ret.p_light = p_light.robust_pos(p_ref.pos - p_light.pos);
        return ret;
    }

    void prepare() noexcept override {
        warper_ = scene().load_warper();
        vector<float> weights = instance()->surface_areas();
        warper_->allocate(weights.size());
        warper_->build(std::move(weights));
        warper_->upload_immediately();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, AreaLight)
//VS_REGISTER_CURRENT_PATH(0, "vision-light-area.dll")