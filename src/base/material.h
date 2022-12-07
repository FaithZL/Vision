//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "node.h"
#include "interaction.h"
#include "core/stl.h"
#include "scattering.h"
#include "texture.h"

namespace vision {

struct BSDF {
public:
    UVN<Float3> shading_frame;
    Float3 ng;
    Float3 world_wo;

protected:
    [[nodiscard]] virtual ScatterEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept {
        ScatterEval ret;
        ret.f = make_float3(0.f);
        ret.pdf = 1.f;
        return ret;
    }

    [[nodiscard]] virtual BSDFSample sample_local(Float3 wo, Uchar flag, Sampler *sampler) const noexcept {
        BSDFSample ret;
        return ret;
    }

public:
    BSDF() = default;
    explicit BSDF(const Interaction &si)
        : shading_frame(si.s_uvn), ng(si.g_uvn.normal()), world_wo(si.wo) {}
    [[nodiscard]] virtual Float3 albedo() const noexcept { return make_float3(0.f); }
    [[nodiscard]] static Uchar combine_flag(Float3 wo, Float3 wi, Uchar flag) noexcept;
    [[nodiscard]] virtual ScatterEval evaluate(Float3 world_wi, Uchar flag) const noexcept;
    [[nodiscard]] ScatterEval evaluate(Float3 world_wi) const noexcept;
    [[nodiscard]] ScatterEval evaluate(Float3 world_wo, Float3 world_wi) const noexcept;
    [[nodiscard]] BSDFSample sample(Sampler *sampler) const noexcept;
    [[nodiscard]] BSDFSample sample(Float3 world_wo, Sampler *sampler) const noexcept;
};

class Material : public Node {
public:
    using Desc = MaterialDesc;

public:
    explicit Material(const MaterialDesc &desc) : Node(desc) {}
    [[nodiscard]] virtual UP<BSDF> get_BSDF(const Interaction &si) const noexcept {
        return make_unique<BSDF>(si);
    }
};
}// namespace vision