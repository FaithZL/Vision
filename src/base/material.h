//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "node.h"
#include "interaction.h"
#include "core/stl.h"
#include "bxdf.h"

namespace vision {

struct BSDF {
private:
    vector<UP<BxDF>> _bxdfs;

public:
    UVN<Float3> shading_frame;
    Float3 ng;

protected:
    [[nodiscard]] virtual Float PDF_(Float3 wo, Float3 wi, Uchar flag) const noexcept;
    [[nodiscard]] virtual Float3 eval_(Float3 wo, Float3 wi,Uchar flag) const noexcept;
    [[nodiscard]] virtual BSDFSample sample_(Float3 wo, Float uc, Float2 u,Uchar flag) const noexcept;

public:
    BSDF() = default;
    explicit BSDF(const SurfaceInteraction &si)
        : shading_frame(si.s_uvn), ng(si.g_uvn.normal()) {}

    template<typename T, typename ...Args>
    void emplace_bxdf(Args &&...args) noexcept {
        _bxdfs.push_back(make_unique<T>(OC_FORWARD(args)...));
    }

    void add_bxdf(UP<BxDF> bxdf) noexcept {
        _bxdfs.push_back(std::move(bxdf));
    }

    [[nodiscard]] virtual Float PDF(Float3 world_wo, Float3 world_wi,Uchar flag) const noexcept  {
        Float3 wo = shading_frame.to_local(world_wo);
        Float3 wi = shading_frame.to_local(world_wi);
        return PDF_(wo, wi, flag);
    }
    [[nodiscard]] virtual Float3 eval(Float3 world_wo, Float3 world_wi,Uchar flag) const noexcept {
        Float3 wo = shading_frame.to_local(world_wo);
        Float3 wi = shading_frame.to_local(world_wi);
        return eval_(wo, wi,flag);
    }
    [[nodiscard]] virtual BSDFSample sample(Float3 world_wo, Float uc, Float2 u,Uchar flag) const noexcept {
        Float3 wo = shading_frame.to_local(world_wo);
        BSDFSample ret = sample_(wo, uc, u,flag);
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