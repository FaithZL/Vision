//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "node.h"
#include "interaction.h"
#include "core/stl.h"
#include "bxdf.h"
#include "texture.h"
#include "math/sampling.h"

namespace vision {

struct BSDF {
public:
    UVN<Float3> shading_frame;
    Float3 ng;
    Float3 world_wo;

protected:
    [[nodiscard]] virtual BSDFSample sample_local(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept {
        BSDFSample ret;
        return ret;
    }
    [[nodiscard]] virtual BSDFEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept {
        BSDFEval ret;
        ret.f = make_float3(0.f);
        ret.pdf = 1.f;
        return ret;
    }

public:
    BSDF() = default;
    explicit BSDF(const SurfaceInteraction &si)
        : shading_frame(si.s_uvn), ng(si.g_uvn.normal()), world_wo(si.wo) {}
    [[nodiscard]] virtual Float3 albedo() const noexcept { return make_float3(0.f); }
    [[nodiscard]] static Uchar combine_flag(Float3 wo, Float3 wi, Uchar flag) noexcept;
    [[nodiscard]] virtual BSDFEval evaluate(Float3 world_wi, Uchar flag) const noexcept;
    [[nodiscard]] virtual BSDFSample sample(Float uc, Float2 u, Uchar flag) const noexcept;
};

class Material : public Node {
public:
    using Desc = MaterialDesc;

public:
    explicit Material(const MaterialDesc &desc) : Node(desc) {}
    template<typename T = float4>
    [[nodiscard]] static Float4 eval_tex(const Texture *tex, const TextureEvalContext &ctx,
                                         T val = T{}) noexcept {
        float4 default_val = make_float4(0);
        if constexpr (is_scalar_v<T>) {
            default_val = make_float4(val);
        } else if constexpr (is_vector2_v<T>) {
            default_val = make_float4(val, 0, 0);
        } else if constexpr (is_vector3_v<T>) {
            default_val = make_float4(val, 0);
        } else {
            default_val = val;
        }
        return tex ? tex->eval(ctx) : Float4(val);
    }
    [[nodiscard]] virtual UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept {
        return make_unique<BSDF>(si);
    }
};
}// namespace vision