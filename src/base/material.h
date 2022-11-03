//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "node.h"
#include "interaction.h"
#include "core/stl.h"
#include "bxdf.h"
#include "math/sampling.h"

namespace vision {

struct BSDF {
public:
    UVN<Float3> shading_frame;
    Float3 ng;

protected:
    [[nodiscard]] virtual BSDFSample sample_local(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept = 0;
    [[nodiscard]] virtual BSDFEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept = 0;

public:
    BSDF() = default;
    explicit BSDF(const SurfaceInteraction &si)
        : shading_frame(si.s_uvn), ng(si.g_uvn.normal()) {}
    [[nodiscard]] virtual Float3 albedo() const noexcept = 0;
    [[nodiscard]] static Uchar combine_flag(Float3 wo, Float3 wi, Uchar flag) noexcept;
    [[nodiscard]] virtual BSDFEval evaluate(Float3 world_wo, Float3 world_wi, Uchar flag) const noexcept;
    [[nodiscard]] virtual BSDFSample sample(Float3 world_wo, Float uc, Float2 u, Uchar flag) const noexcept;
};

class Material : public Node {
public:
    using Desc = MaterialDesc;

public:
    explicit Material(const MaterialDesc &desc) : Node(desc) {}
    [[nodiscard]] virtual UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept = 0;
};
}// namespace vision