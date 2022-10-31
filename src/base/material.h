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
protected:
    vector<UP<BxDF>> _bxdfs;

public:
    UVN<Float3> shading_frame;
    Float3 ng;

protected:
    [[nodiscard]] virtual BSDFSample sample_local(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept;
    [[nodiscard]] virtual Evaluation evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept;
    template<typename Func>
    void for_each(Func &&func) noexcept {
        for (UP<BxDF> &bxdf : _bxdfs) {
            func(bxdf.get());
        }
    }
    template<typename Func>
    void for_each(Func &&func) const noexcept {
        for (const UP<BxDF> &bxdf : _bxdfs) {
            func(bxdf.get());
        }
    }
public:
    BSDF() = default;
    explicit BSDF(const SurfaceInteraction &si)
        : shading_frame(si.s_uvn), ng(si.g_uvn.normal()) {}
    template<typename T, typename... Args>
    void emplace_bxdf(Args &&...args) noexcept { _bxdfs.push_back(make_unique<T>(OC_FORWARD(args)...)); }
    void add_bxdf(UP<BxDF> bxdf) noexcept { _bxdfs.push_back(std::move(bxdf)); }
    [[nodiscard]] Int match_num(Uchar bxdf_flag) const noexcept;
    [[nodiscard]] Uchar flag() const noexcept;
    [[nodiscard]] Float3 albedo() const noexcept { return _bxdfs[0]->albedo(); }
    [[nodiscard]] static Uchar combine_flag(Float3 wo, Float3 wi, Uchar flag) noexcept;
    [[nodiscard]] virtual Evaluation evaluate(Float3 world_wo, Float3 world_wi, Uchar flag) const noexcept;
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