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
//    Warper2D warper;
    float4x4 _o2w;
    float _scale{1.f};
    Texture *_texture{nullptr};

public:
    explicit EnvironmentLight(const LightDesc &desc)
        : Light(desc, LightType::Infinite),
          _o2w(desc.o2w.mat),
          _scale(desc.scale),
          _texture(desc.scene->load<Texture>(desc.texture_desc)) {}

    [[nodiscard]] Float3 Li(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept override {
        // todo
        return make_float3(0.f);
    }

    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept override {
        // todo
        return 0.f;
    }

    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u) const noexcept override {
        // todo
        return LightSample{};
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
            weights[idx] = f * sinTheta;
        });
        return weights;
    }

    void prepare(RenderPipeline *rp) noexcept override {
        vector<float> weights = calculate_weights();
//        warper.build(rp, weights, _texture->resolution());
//        warper.prepare(rp);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::EnvironmentLight)