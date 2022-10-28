//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "node.h"
#include "sample.h"
#include "interaction.h"
#include "core/stl.h"

namespace vision {

struct BSDF {
public:
    UVN<Float3> shading_frame;
    Float3 ng;

protected:
    [[nodiscard]] virtual Float PDF_(Float3 wo, Float3 wi) const noexcept = 0;
    [[nodiscard]] virtual Float3 eval_(Float3 wo, Float3 wi) const noexcept = 0;
    [[nodiscard]] virtual BSDFSample sample_(Float3 wo, Float uc, Float2 u) const noexcept = 0;
public:
    BSDF() = default;
    explicit BSDF(const SurfaceInteraction &si)
        : shading_frame(si.s_uvn), ng(si.g_uvn.normal()) {}

    [[nodiscard]] virtual Float PDF(Float3 world_wo, Float3 world_wi) const noexcept  {
        Float3 wo = shading_frame.to_local(world_wo);
        Float3 wi = shading_frame.to_local(world_wi);
        return PDF_(wo, wi);
    }
    [[nodiscard]] virtual Float3 eval(Float3 world_wo, Float3 world_wi) const noexcept {
        Float3 wo = shading_frame.to_local(world_wo);
        Float3 wi = shading_frame.to_local(world_wi);
        return eval_(wo, wi);
    }
    [[nodiscard]] virtual BSDFSample sample(Float3 world_wo, Float uc, Float2 u) const noexcept {
        Float3 wo = shading_frame.to_local(world_wo);
        BSDFSample ret = sample_(wo, uc, u);
        ret.wi = shading_frame.to_local(ret.wi);
        ret.val *= abs_dot(shading_frame.z, ret.wi);
        return ret;
    }
};

class Material : public Node {
public:
    using Desc = MaterialDesc;

public:
    explicit Material(const MaterialDesc &desc) : Node(desc) {}
    [[nodiscard]] virtual UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept = 0;
};
}// namespace vision