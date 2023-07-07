//
// Created by Zero on 2023/7/7.
//

#pragma once

#include "math/warp.h"
#include "base/warper.h"
#include "base/mgr/global.h"
#include "base/sensor/filter.h"
#include "base/mgr/pipeline.h"

namespace vision {

class FilterSampler {
public:
    static constexpr int table_size = 20;

private:
    Warper2D *_warper{};
    RegistrableManaged<float> _lut;

public:
    FilterSampler()
        : _warper(Global::instance().pipeline()->scene().load_warper2d()) {}

    [[nodiscard]] FilterSample sample(Float2 u) const noexcept {
        return {};
    }
};

class FittedCurveFilter : public Filter {
protected:
    FilterSampler _sampler{};

public:
    explicit FittedCurveFilter(const FilterDesc &desc)
        : Filter(desc) {}

    [[nodiscard]] FilterSample sample(Float2 u) const noexcept override {
        return _sampler.sample(u);
    }
};

}// namespace vision