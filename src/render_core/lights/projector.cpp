//
// Created by Zero on 27/11/2022.
//

#include "base/illumination/light.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/pipeline.h"

namespace vision {

//    "type" : "projector",
//    "param" : {
//        "angle" : 6,
//        "color" : {
//            "channels" : "xyz",
//            "node" : {
//                "fn" : "Painting3.jpg",
//                "color_space" : "srgb"
//            }
//        },
//        "ratio" : 1,
//        "o2w" : {
//            "type" : "look_at",
//            "param" : {
//                "position" : [0,1,6.8],
//                "up" : [0,1,0],
//                "target_pos" : [0,1,0]
//            }
//        },
//        "scale" : 300
//    }
class Projector : public IPointLight {
private:
    EncodedData<float4x4> o2w_;
    EncodedData<float> ratio_;
    EncodedData<float> angle_y_;

public:
    Projector() = default;
    explicit Projector(const LightDesc &desc)
        : IPointLight(desc),
          ratio_(desc["ratio"].as_float(1.f)),
          angle_y_(radians(ocarina::clamp(desc["angle"].as_float(45.f), 1.f, 89.f))),
          o2w_(desc.o2w.mat) {
        if (ratio_.hv() == 0) {
            uint2 res = color_->resolution();
            ratio_ = float(res.x) / res.y;
        }
    }
    OC_ENCODABLE_FUNC(IPointLight, o2w_, ratio_, angle_y_)
    VS_HOTFIX_MAKE_RESTORE(IPointLight, o2w_, ratio_, angle_y_)
    VS_MAKE_PLUGIN_NAME_FUNC
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        IPointLight::render_sub_UI(widgets);
        changed_ |= widgets->input_float_limit("radio", addressof(ratio_.hv()), 0.1, 10, 0.05, 0.2);
        changed_ |= widgets->slider_float("angle_y_", &angle_y_.hv(), radians(1.f), radians(89.f));
    }
    [[nodiscard]] float3 power() const noexcept override {
        // http://math.stackexchange.com/questions/9819/area-of-a-spherical-triangle
        // Girard's theorem: surface area of a spherical triangle on a unit
        // sphere is the 'excess angle' alpha+beta+gamma-pi, where
        // alpha/beta/gamma are the interior angles at the vertices.
        //
        // Given three vertices on the sphere, a, b, c, then we can compute,
        // for example, the angle c->a->b by
        //
        // cos theta =  Dot(Cross(c, a), Cross(b, a)) /
        //              (Length(Cross(c, a)) * Length(Cross(b, a))).
        //
        using ocarina::clamp;
        using ocarina::sqr;
        using ocarina::sqrt;
        float y = sqrt(1.f / (sqr(ratio_.hv()) + 1.f));
        float x = ratio_.hv() * y;
        float z = sqrt(sqr(x) + sqr(y));
        float3 p0 = make_float3(x, y, z);
        float3 p1 = make_float3(x, -y, z);
        float3 p2 = make_float3(-x, -y, z);
        float3 cross01 = cross(p0, p1);
        float3 cross12 = cross(p1, p2);
        float3 cross20 = cross(p2, p0);
        using namespace ocarina;
        if (length_squared(cross01) > 0) cross01 = normalize(cross01);
        if (length_squared(cross12) > 0) cross12 = normalize(cross12);
        if (length_squared(cross20) > 0) cross20 = normalize(cross20);
        float solid_angle = ocarina::abs(
            ocarina::acos(clamp(dot(cross01, -cross12), -1.f, 1.f)) +
            ocarina::acos(clamp(dot(cross12, -cross20), -1.f, 1.f)) +
            ocarina::acos(clamp(dot(cross20, -cross01), -1.f, 1.f)) - Pi);
        return (2 * solid_angle) / (4 * Pi) * average();
    }
    [[nodiscard]] Float3 position() const noexcept override { return (*o2w_)[3].xyz(); }
    [[nodiscard]] float3 &host_position() noexcept override {
        return reinterpret_cast<float3 &>(o2w_.hv()[3]);
    }
    [[nodiscard]] Float3 direction(const LightSampleContext &p_ref) const noexcept override {
        return transform_vector<D>(*o2w_, make_float3(0, 0, 1));
    }
    [[nodiscard]] SampledSpectrum Le(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        Float3 p = transform_point(inverse(*o2w_), p_ref.pos);
        Float d2 = length_squared(p);
        Bool valid = p.z > 0;
        p = p / p.z;
        Float tan_y = tan(*angle_y_);
        Float tan_x = *ratio_ * tan_y;
        Float2 tan_xy = make_float2(tan_x, tan_y);
        Float2 uv = (p.xy() + tan_xy) / (2.f * tan_xy);
        valid = valid && all(uv >= 0.f && uv <= 1.f);
        return select(valid, 1.f, 0.f) * color_.eval_illumination_spectrum(uv, swl).sample / d2 * scale();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, Projector)