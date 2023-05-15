//
// Created by Zero on 27/11/2022.
//

#include "base/light.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/render_pipeline.h"

namespace vision {
class Projector : public IPointLight {
private:
    Serial<float4x4> _o2w;
    Serial<float> _ratio;
    Serial<float> _angle_y;

public:
    explicit Projector(const LightDesc &desc)
        : IPointLight(desc),
          _ratio(desc["ratio"].as_float(1.f)),
          _angle_y(radians(ocarina::clamp(desc["angle"].as_float(45.f), 1.f, 89.f))),
          _o2w(desc.o2w.mat) {
        if (_ratio.hv() == 0) {
            uint2 res = _color.node()->resolution();
            _ratio = float(res.x) / res.y;
        }
    }
    OC_SERIALIZABLE_FUNC(_o2w, _ratio, _angle_y)
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
        float y = sqrt(1.f / (sqr(_ratio.hv()) + 1.f));
        float x = _ratio.hv() * y;
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
    [[nodiscard]] Float3 position() const noexcept override { return (*_o2w)[3].xyz(); }
    [[nodiscard]] SampledSpectrum Li(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        Float3 p = transform_point(inverse(*_o2w), p_ref.pos);
        Float d2 = length_squared(p);
        Bool valid = p.z > 0;
        p = p / p.z;
        Float tan_y = tan(*_angle_y);
        Float tan_x = *_ratio * tan_y;
        Float2 tan_xy = make_float2(tan_x, tan_y);
        Float2 uv = (p.xy() + tan_xy) / (2.f * tan_xy);
        valid = valid && all(uv >= 0.f && uv <= 1.f);
        return select(valid, 1.f, 0.f) * _color.eval_illumination_spectrum(uv, swl).sample / d2 * scale();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Projector)