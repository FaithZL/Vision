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
    Warper2D warper;
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
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::EnvironmentLight)