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
    float4x4 o2w_;
    EncodedData<float4x4> w2o_;
    SP<Warper2D> warper_{};
    float yaw_{0};
    bool flip_u_{false};

public:
    SphericalMap() : Environment(LightType::Infinite) {}
    explicit SphericalMap(const Desc &desc)
        : Environment(desc, LightType::Infinite),
          flip_u_(desc["flip_u"].as_bool(false)) {
        float4x4 o2w = desc.o2w.mat;
        float x = flip_u_ ? 1.f : -1.f;
        float4x4 rx = rotation_x<H>(-90) * transform::scale(1.f, x, 1.f);
        o2w_ = o2w * rx;
        w2o_ = inverse(o2w_);
    }
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        Environment::render_sub_UI(widgets);
        changed_ |= widgets->drag_float("yaw", std::addressof(yaw_), 1, -180, 180);
        if (changed_) {
            float4x4 r = rotation_y<H>(yaw_);
            w2o_ = inverse(r * o2w_);
        }
    }
    OC_ENCODABLE_FUNC(Environment, w2o_, *warper_)
    VS_HOTFIX_MAKE_RESTORE(Environment, o2w_, w2o_, warper_, yaw_, flip_u_)
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] float3 power() const noexcept override {
        float world_radius = scene().world_diameter() / 2.f;
        return Pi * ocarina::sqr(world_radius) * average();
    }
    [[nodiscard]] static Float2 calculate_uv(const Float3& local_dir) noexcept {
        Float org_u = spherical_phi(local_dir) * Inv2Pi;
        return make_float2(org_u, spherical_theta(local_dir) * InvPi);
    }

    [[nodiscard]] SampledSpectrum L(const Float3 &local_dir, const SampledWavelengths &swl) const {
        Float2 uv = calculate_uv(local_dir);
        return color_.eval_illumination_spectrum(uv, swl).sample * scale();
    }

    [[nodiscard]] Float2 convert_to_bary(const Float3 &world_dir) const noexcept override {
        Float3 local_dir = normalize(transform_vector(*w2o_, world_dir));
        return calculate_uv(local_dir);
    }

    [[nodiscard]] SampledSpectrum Le(const LightSampleContext &p_ref, const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        OC_ERROR("Spherical Le can not be called");
        return {swl.dimension(), 0.f};
    }

    [[nodiscard]] Float PDF_wi(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept override {
        OC_ERROR("Spherical PDF_wi can not be called");
        return 0;
    }

    [[nodiscard]] LightEval evaluate_wi(const LightSampleContext &p_ref, const LightEvalContext &p_light,
                                        const SampledWavelengths &swl, LightEvalMode mode) const noexcept override {
        LightEval ret{swl.dimension()};
        Float3 world_dir = normalize(p_light.pos - p_ref.pos);
        Float3 local_dir = transform_vector(*w2o_, world_dir);
        Float theta = spherical_theta(local_dir);
        Float phi = spherical_phi(local_dir);
        Float sin_theta = sin(theta);
        Float2 uv = make_float2(phi * Inv2Pi, theta * InvPi);
        if (match_L(mode)) {
            ret.L = L(local_dir, swl);
        }
        if (match_PDF(mode)) {
            Float pdf = warper_->PDF(uv) / (_2Pi * Pi * sin_theta);
            ret.pdf = select(sin_theta == 0, 0.f, pdf);
        }
        return ret;
    }

    [[nodiscard]] LightSample evaluate(const LightSampleContext &p_ref, Float2 uv, Float pdf_map,
                                       const SampledWavelengths &swl, LightEvalMode mode) const noexcept {
        LightSample ret{swl.dimension()};
        Float theta = uv[1] * Pi;
        Float phi = uv[0] * _2Pi;
        Float sin_theta = sin(theta);
        Float cos_theta = cos(theta);
        Float3 local_dir = spherical_direction(sin_theta, cos_theta, phi);
        Float3 world_dir = normalize(transform_vector(inverse(*w2o_), local_dir));
        if (match_PDF(mode)) {
            Float pdf_dir = pdf_map / (_2Pi * Pi * sin_theta);
            pdf_dir = select(ocarina::isinf(pdf_dir), 0.f, pdf_dir);
            ret.eval.pdf = pdf_dir;
        }
        if (match_L(mode)) {
            ret.eval.L = L(local_dir, swl);
        }
        Float3 pos = p_ref.pos + world_dir * scene().world_diameter();
        ret.p_light = pos;
        return ret;
    }

    [[nodiscard]] LightSurfacePoint sample_only(ocarina::Float2 u) const noexcept override {
        LightSurfacePoint lsp;
        Float2 uv = warper_->sample_continuous(u, nullptr, nullptr);
        lsp.bary = uv;
        return lsp;
    }

    [[nodiscard]] LightSample evaluate_point(const LightSampleContext &p_ref, LightSurfacePoint lsp,
                                             const SampledWavelengths &swl, LightEvalMode mode) const noexcept override {
        Float pdf_map = 0.f;
        if (match_PDF(mode)) {
            pdf_map = warper_->PDF(lsp.bary);
        }
        return evaluate(p_ref, lsp.bary, pdf_map, swl, mode);
    }

    [[nodiscard]] Float PDF_point(const LightSampleContext &p_ref, const LightEvalContext &p_light,
                                  const Float &pdf_wi) const noexcept override {
        return pdf_wi;
    }

    [[nodiscard]] LightEvalContext compute_light_eval_context(const LightSampleContext &p_ref,
                                                              LightSurfacePoint lsp) const noexcept override {
        Float pdf_map = warper_->PDF(lsp.bary);
        Float2 uv = lsp.bary;
        Float theta = uv[1] * Pi;
        Float phi = uv[0] * _2Pi;
        Float sin_theta = sin(theta);
        Float cos_theta = cos(theta);
        Float3 local_dir = spherical_direction(sin_theta, cos_theta, phi);
        Float3 world_dir = normalize(transform_vector(inverse(*w2o_), local_dir));
        Float3 pos = p_ref.pos + world_dir * scene().world_diameter();
        return LightEvalContext{pos, -world_dir};
    }

    [[nodiscard]] LightSample sample_wi(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override {
        Float pdf_map;
        Float2 uv = warper_->sample_continuous(u, std::addressof(pdf_map),
                                               nullptr);
        return evaluate(p_ref, uv, pdf_map, swl, LightEvalMode::All);
    }

    [[nodiscard]] vector<float> calculate_weights() noexcept {
        uint2 res = color_->resolution();
        vector<float> weights(res.x * res.y, 0);
        color_->for_each_pixel([&](const std::byte *pixel, int idx, PixelStorage pixel_storage) {
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
        warper_ = scene().load_warper2d();
        uint2 res = color_->resolution();
        vector<float> weights;
        if (any(res == 0u)) {
            res = make_uint2(1);
            weights.push_back(1.f);
        } else {
            weights = calculate_weights();
        }
        warper_->allocate(res);
        warper_->build(weights, res);
        warper_->upload_immediately();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, SphericalMap)
//VS_REGISTER_CURRENT_PATH(0, "vision-light-spherical.dll")