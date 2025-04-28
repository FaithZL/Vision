//
// Created by Zero on 2025/4/28.
//

#include "base/sensor/tonemapper.h"
#include "hotfix/hotfix.h"

namespace vision {

class LinearToneMapper : public ToneMapper {
public:
    LinearToneMapper() = default;
    explicit LinearToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC_(linear)
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return input;
    }
};

class ACESToneMapper : public ToneMapper {
public:
    ACESToneMapper() = default;
    explicit ACESToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC_(aces)
    [[nodiscard]] Float4 apply(const ocarina::Float4 &x) const noexcept override {
        float a = 2.51f;
        float b = 0.03f;
        float c = 2.43f;
        float d = 0.59f;
        float e = 0.14f;
        return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
    }
};

class ReinhardToneMapper : public ToneMapper {
public:
    ReinhardToneMapper() = default;
    explicit ReinhardToneMapper(const ToneMapperDesc &desc)
        : ToneMapper(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC_(reinhard)
    [[nodiscard]] Float4 apply(const ocarina::Float4 &input) const noexcept override {
        return input / (input + 1.f);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, LinearToneMapper, linear)
VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, ACESToneMapper, aces)
VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, ReinhardToneMapper, reinhard)
VS_REGISTER_CURRENT_PATH(0, "vision-tonemapper-impl.dll")