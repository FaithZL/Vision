//
// Created by Zero on 29/11/2022.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"
#include "base/shader_graph/shader_node.h"
#include "math/warp.h"

namespace vision {

//    "type" : "spherical",
//    "param": {
//        "color": {
//            "fn" : "textures/spruit_sunrise_2k.hdr",
//            "color_space": "linear"
//        },
//        "o2w" : {
//            "type":"Euler",
//            "param": {
//                "yaw" :-30
//            }
//        },
//        "scale" : 3
//    }
class SphericalMap : public Environment {
private:
    Serial<float4x4> _w2o;
    SP<Warper2D> _warper{};

public:
    explicit SphericalMap(const Desc &desc)
        : Environment(desc, LightType::Infinite) {
        float4x4 o2w = desc.o2w.mat;
        float4x4 rx = rotation_x<H>(-90);
        _w2o = inverse(o2w * rx);
    }

    OC_SERIALIZABLE_FUNC(Environment, _w2o, *_warper)
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    [[nodiscard]] float3 power() const noexcept override {
        float world_radius = scene().world_diameter() / 2.f;
        return Pi * ocarina::sqr(world_radius) * average();
    }
    [[nodiscard]] static Float2 UV(Float3 local_dir) {
        return make_float2(spherical_phi(local_dir) * Inv2Pi, spherical_theta(local_dir) * InvPi);
    }

    [[nodiscard]] SampledSpectrum L(Float3 local_dir, const SampledWavelengths &swl) const {
        Float2 uv = UV(local_dir);
        return _color.eval_illumination_spectrum(uv, swl).sample * scale();
    }

    [[nodiscard]] Float2 convert_to_bary(const Float3 &world_dir) const noexcept override {
        Float3 local_dir = normalize(transform_vector(*_w2o, world_dir));
        return UV(local_dir);
    }

    [[nodiscard]] SampledSpectrum Le(const LightSampleContext &p_ref, const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        OC_ERROR("Spherical Le can not be called");
        return {swl.dimension(), 0.f};
    }

    [[nodiscard]] Float PDF_wi(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept override {
        OC_ERROR("Spherical PDF_wi can not be called")
        return 0;
    }

    [[nodiscard]] LightEval evaluate_wi(const LightSampleContext &p_ref, const LightEvalContext &p_light,
                                        const SampledWavelengths &swl, LightEvalMode mode) const noexcept override {
        Float3 world_dir = normalize(p_light.pos - p_ref.pos);
        Float3 local_dir = transform_vector(*_w2o, world_dir);
        Float theta = spherical_theta(local_dir);
        Float phi = spherical_phi(local_dir);
        Float sin_theta = sin(theta);
        Float2 uv = make_float2(phi * Inv2Pi, theta * InvPi);
        Float pdf = _warper->PDF(uv) / (_2Pi * Pi * sin_theta);
        return {L(local_dir, swl), select(sin_theta == 0, 0.f, pdf)};
    }

    [[nodiscard]] LightSample evaluate(const LightSampleContext &p_ref, Float2 uv, Float pdf_map,
                                       const SampledWavelengths &swl, LightEvalMode mode) const noexcept {
        LightSample ret{swl.dimension()};
        Float theta = uv[1] * Pi;
        Float phi = uv[0] * _2Pi;
        Float sin_theta = sin(theta);
        Float cos_theta = cos(theta);
        Float3 local_dir = spherical_direction(sin_theta, cos_theta, phi);
        Float3 world_dir = normalize(transform_vector(inverse(*_w2o), local_dir));
        Float pdf_dir = pdf_map / (_2Pi * Pi * sin_theta);
        pdf_dir = select(ocarina::isinf(pdf_dir), 0.f, pdf_dir);
        ret.eval = LightEval(L(local_dir, swl), pdf_dir);
        Float3 pos = p_ref.pos + world_dir * scene().world_diameter();
        ret.p_light = pos;
        return ret;
    }

    [[nodiscard]] LightSurfacePoint sample_only(ocarina::Float2 u) const noexcept override {
        LightSurfacePoint lsp;
        Float2 uv = _warper->sample_continuous(u, nullptr, nullptr);
        lsp.bary = uv;
        return lsp;
    }

    [[nodiscard]] LightSample evaluate_point(const LightSampleContext &p_ref, LightSurfacePoint lsp,
                                             const SampledWavelengths &swl, LightEvalMode mode) const noexcept override {
        Float pdf_map = _warper->PDF(lsp.bary);
        return evaluate(p_ref, lsp.bary, pdf_map, swl, mode);
    }

    [[nodiscard]] Float PDF_point(const LightSampleContext &p_ref, const LightEvalContext &p_light,
                                  const Float &pdf_wi) const noexcept override {
        return pdf_wi;
    }

    [[nodiscard]] LightEvalContext compute_light_eval_context(const LightSampleContext &p_ref,
                                                              LightSurfacePoint lsp) const noexcept override {
        Float pdf_map = _warper->PDF(lsp.bary);
        Float2 uv = lsp.bary;
        Float theta = uv[1] * Pi;
        Float phi = uv[0] * _2Pi;
        Float sin_theta = sin(theta);
        Float cos_theta = cos(theta);
        Float3 local_dir = spherical_direction(sin_theta, cos_theta, phi);
        Float3 world_dir = normalize(transform_vector(inverse(*_w2o), local_dir));
        Float3 pos = p_ref.pos + world_dir * scene().world_diameter();
        return LightEvalContext{pos, -world_dir};
    }

    [[nodiscard]] LightSample sample_wi(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override {
        Float pdf_map;
        Float2 uv = _warper->sample_continuous(u, std::addressof(pdf_map),
                                               nullptr);
        return evaluate(p_ref, uv, pdf_map, swl, LightEvalMode::All);
    }

    [[nodiscard]] vector<float> calculate_weights() noexcept {
        uint2 res = _color.node()->resolution();
        vector<float> weights(res.x * res.y, 0);
        _color.node()->for_each_pixel([&](const std::byte *pixel, int idx, PixelStorage pixel_storage) {
            float f = 0;
            float v = idx / res.y + 0.5f;
            float theta = v / res.x;
            float sinTheta = std::sin(Pi * theta);
            switch (pixel_storage) {
                case PixelStorage::FLOAT4: {
                    float4 val = *(reinterpret_cast<const float4 *>(pixel));
                    f = luminance(val.xyz());
                    break;
                }
                case PixelStorage::BYTE4: {
                    uchar4 val = *(reinterpret_cast<const uchar4 *>(pixel));
                    float4 f4 = make_float4(val) / 255.f;
                    f = luminance(f4.xyz());
                    break;
                }
                default:
                    break;
            }
            weights[idx] = f * sinTheta;
        });
        return weights;
    }

    void prepare() noexcept override {
        _warper = scene().load_warper2d();
        uint2 res = _color.node()->resolution();
        vector<float> weights;
        if (any(res == 0u)) {
            res = make_uint2(1);
            weights.push_back(1.f);
        } else {
            weights = calculate_weights();
        }
        _warper->build(weights, res);
        _warper->prepare();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SphericalMap)