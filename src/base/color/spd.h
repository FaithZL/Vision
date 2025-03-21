//
// Created by Zero on 26/12/2022.
//

#pragma once

#include "dsl/dsl.h"
#include "rhi/common.h"
#include "base/mgr/pipeline.h"
#include "cie.h"
#include "spectrum.h"

namespace vision {

using namespace ocarina;

class SPD : public Encodable<encoded_ty>{
private:
    static constexpr auto spd_lut_interval = 5u;
    RegistrableManaged<float> func_;
    EncodedData<float> sample_interval_{};
    Pipeline *rp_{};

public:
    SPD() = default;
    explicit SPD(Pipeline *rp);
    SPD(vector<float> func, Pipeline *rp);
    OC_ENCODABLE_FUNC(Encodable<encoded_ty>, func_, sample_interval_)

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
    void upload() noexcept;
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
    [[nodiscard]] DynamicArray<float> eval(const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] static SPD create_cie_x(Pipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_y(Pipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_z(Pipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_d65(Pipeline *rp) noexcept;
    [[nodiscard]] static float cie_y_integral() noexcept;
};

}// namespace vision