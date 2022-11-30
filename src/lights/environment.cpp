//
// Created by Zero on 29/11/2022.
//

#include "base/light.h"
#include "base/render_pipeline.h"
#include "base/texture.h"
#include "math/warp.h"

namespace vision {

class EnvironmentLight : public Light {
private:
    Warper2D *_warper{};
    float4x4 _w2o;
    float _scale{1.f};
    Texture *_texture{nullptr};

public:
    explicit EnvironmentLight(const LightDesc &desc)
        : Light(desc, LightType::Infinite),
          _scale(desc.scale),
          _texture(desc.scene->load<Texture>(desc.texture_desc)) {
        float4x4 o2w = desc.o2w.mat;
        float4x4 rx = rotation_x<H>(90);
        _w2o = inverse(o2w * rx);
    }

    [[nodiscard]] Float2 UV(Float3 local_dir) const {
        return make_float2(spherical_phi(local_dir) * Inv2Pi, spherical_theta(local_dir) * InvPi);
    }

    [[nodiscard]] Float3 L(Float3 local_dir) const {
        Float2 uv = UV(local_dir);
        return _texture->eval(uv).xyz();
    }

    [[nodiscard]] Float3 Li(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept override {
        Float3 direction = normalize(p_light.pos - p_ref.pos);
        return L(transform_vector(_w2o, direction));
    }

    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept override {
        Float3 direction = normalize(p_light.pos - p_ref.pos);
        direction = transform_vector(_w2o, direction);
        Float theta = spherical_theta(direction);
        Float phi = spherical_phi(direction);
        Float sin_theta = sin(theta);
        Float2 uv = make_float2(phi * Inv2Pi, theta * InvPi);
        Float pdf = _warper->PDF(uv) / (_2Pi * Pi * sin_theta);
        return select(sin_theta == 0, 0.f, pdf);
    }

    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u) const noexcept override {
        LightSample ret;
        auto [uv, pdf_map, coord] = _warper->sample_continuous(u);
        Float theta = uv[1] * Pi;
        Float phi = uv[0] * _2Pi;
        Float sin_theta = sin(theta);
        Float cos_theta = cos(theta);
        Float3 direction = spherical_direction(sin_theta, cos_theta, phi);
        direction = normalize(transform_vector(inverse(_w2o), direction));
        Float pdf_dir = pdf_map / (_2Pi * Pi * sin_theta);
        Float3 pos = p_ref.pos + direction * _scene->world_range();
        LightEvalContext p_light{pos};
        ret.eval = evaluate(p_ref, p_light);
        ret.p_light = pos;
        return ret;
    }

    [[nodiscard]] vector<float> calculate_weights() noexcept {
        uint2 res = _texture->resolution();
        vector<float> weights(res.x * res.y, 0);
        _texture->for_each_pixel([&](const std::byte *pixel, int idx, PixelStorage pixel_storage) {
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
            f = 1.f;
            weights[idx] = f * sinTheta;
        });
        return weights;
    }

    void prepare(RenderPipeline *rp) noexcept override {
        vector<float> weights = calculate_weights();
        Scene &scene = rp->scene();
        _warper = scene.load_warper2d();
        _warper->build(rp, weights, _texture->resolution());
        _warper->prepare(rp);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::EnvironmentLight)