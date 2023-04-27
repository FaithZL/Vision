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

class SPD : public ISerializable<float>{
private:
    static constexpr auto spd_lut_interval = 5u;
    ManagedWrapper<float> _func;
    Serialize<float> _sample_interval{};
    RenderPipeline *_rp{};

public:
    explicit SPD(RenderPipeline *rp);
    SPD(vector<float> func, RenderPipeline *rp);
    OC_SERIALIZABLE_FUNC(float, _func, _sample_interval)

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
    [[nodiscard]] uint buffer_index() const noexcept { return _func.index().hv(); }
    [[nodiscard]] Float eval(const Uint &index, const Float &lambda) const noexcept;
    [[nodiscard]] float eval(float lambda) const noexcept;
    template<size_t N>
    [[nodiscard]] Vector<float, N> eval(Vector<float, N> lambdas) const noexcept {
        Vector<float, N> ret;
        for (int i = 0; i < N; ++i) {
            ret[i] = eval(lambdas[i]);
        }
        return ret;
    }
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