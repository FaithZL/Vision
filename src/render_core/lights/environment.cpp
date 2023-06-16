//
// Created by Zero on 29/11/2022.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"
#include "base/shader_graph/shader_node.h"
#include "math/warp.h"

namespace vision {

class EnvironmentLight : public Light {
private:
    Serial<float4x4> _w2o;
    Warper2D *_warper{};

public:
    explicit EnvironmentLight(const LightDesc &desc)
        : Light(desc, LightType::Infinite){
        float4x4 o2w = desc.o2w.mat;
        float4x4 rx = rotation_x<H>(-90);
        _w2o = inverse(o2w * rx);
    }

    OC_SERIALIZABLE_FUNC(Light, _w2o, *_warper)

    [[nodiscard]] Float2 UV(Float3 local_dir) const {
        return make_float2(spherical_phi(local_dir) * Inv2Pi, spherical_theta(local_dir) * InvPi);
    }

    [[nodiscard]] SampledSpectrum L(Float3 local_dir,const SampledWavelengths &swl) const {
        Float2 uv = UV(local_dir);
        return _color.eval_illumination_spectrum(uv, swl).sample * scale();
    }

    [[nodiscard]] SampledSpectrum Li(const LightSampleContext &p_ref, const LightEvalContext &p_light,
                             const SampledWavelengths &swl) const noexcept override {
        OC_ERROR("environment PDF_Li can not be called");
        return {3u, 0.f};
    }

    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept override {
        OC_ERROR("environment PDF_Li can not be called")
        return 0;
    }

    [[nodiscard]] LightEval evaluate(const LightSampleContext &p_ref, const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        Float3 world_dir = normalize(p_light.pos - p_ref.pos);
        Float3 local_dir = transform_vector(*_w2o, world_dir);
        Float theta = spherical_theta(local_dir);
        Float phi = spherical_phi(local_dir);
        Float sin_theta = sin(theta);
        Float2 uv = make_float2(phi * Inv2Pi, theta * InvPi);
        Float pdf = _warper->PDF(uv) / (_2Pi * Pi * sin_theta);
        return {L(local_dir,swl), select(sin_theta == 0, 0.f, pdf)};
    }

    [[nodiscard]] float3 power() const noexcept override {
        float world_radius = scene().world_diameter() / 2.f;
        return Pi * ocarina::sqr(world_radius) * average();
    }

    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override {
        LightSample ret{swl.dimension()};

        Float pdf_map;
        Uint2 coord;

        Float2 uv = _warper->sample_continuous(u, &pdf_map, &coord);
        Float theta = uv[1] * Pi;
        Float phi = uv[0] * _2Pi;
        Float sin_theta = sin(theta);
        Float cos_theta = cos(theta);
        Float3 local_dir = spherical_direction(sin_theta, cos_theta, phi);
        Float3 world_dir = normalize(transform_vector(inverse(*_w2o), local_dir));
        Float pdf_dir = pdf_map / (_2Pi * Pi * sin_theta);
        Float3 pos = p_ref.pos + world_dir * scene().world_diameter();
        ret.eval.L = L(local_dir, swl);
        pdf_dir = select(ocarina::isinf(pdf_dir), 0.f, pdf_dir);
        ret.eval.pdf = pdf_dir;
        ret.p_light = pos;
        return ret;
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

VS_MAKE_CLASS_CREATOR(vision::EnvironmentLight)