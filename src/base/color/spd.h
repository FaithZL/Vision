//
// Created by Zero on 26/12/2022.
//

#pragma once

#include "dsl/common.h"
#include "rhi/common.h"
#include "base/mgr/render_pipeline.h"
#include "cie.h"
#include "spectrum.h"

namespace vision {

using namespace ocarina;

// todo change to scalar or vector template
class SPD {
private:
    static constexpr auto spd_lut_interval = 5u;
    ManagedWrapper<float> _func;
    float _sample_interval{};
    RenderPipeline *_rp{};

public:
    explicit SPD(RenderPipeline *rp);
    SPD(vector<float> func, RenderPipeline *rp);

    void init(vector<float> func) noexcept;
    template<typename T>
    requires concepts::iterable<T>
    void init(T &&t) noexcept {
        std::vector<float> lst;
        for (const auto &elm : OC_FORWARD(t)) {
            lst.push_back(elm);
        }
        init(lst);
    }
    template<typename Func>
    static vector<float> to_list(Func &&func, float interval = spd_lut_interval) noexcept {
        vector<float> ret;
        for (float lambda = cie::visible_wavelength_min; lambda < cie::visible_wavelength_max; lambda += interval) {
            ret.push_back(func(lambda));
        }
        return ret;
    }
    void prepare() noexcept;
    [[nodiscard]] uint buffer_index() const noexcept { return _func.index(); }
    [[nodiscard]] Float eval(const Uint &index, const Float &lambda) const noexcept;
    [[nodiscard]] Float eval(const Float& lambdas) const noexcept;
    [[nodiscard]] SampledSpectrum eval(const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] Array<float> eval(const Uint &index, const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] static SPD create_cie_x(RenderPipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_y(RenderPipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_z(RenderPipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_d65(RenderPipeline *rp) noexcept;
    [[nodiscard]] static float cie_y_integral() noexcept;
};

}// namespace vision