//
// Created by Zero on 2023/7/7.
//

#pragma once

#include "math/warp.h"
#include "base/warper.h"
#include "base/mgr/global.h"
#include "base/sensor/filter.h"
#include "base/mgr/pipeline.h"
#include "rhi/common.h"

namespace vision {
using namespace ocarina;
class FilterSampler : public Ctx {
public:
    static constexpr int table_size = 20;

private:
    Warper2D *_warper{};
    RegistrableManaged<float> _lut{pipeline()->resource_array()};

public:
    FilterSampler()
        : _warper(scene().load_warper2d()) {}

    void prepare(const Filter *filter) {
        int len = ocarina::sqr(table_size);
        _lut.resize(len);
        vector<float> func;
        func.resize(len);
        float2 r = filter->radius<H>();
        for (int i = 0; i < len; ++i) {
            int x = i % table_size;
            int y = i / table_size;
            float2 p = make_float2((x + 0.5f) / table_size * r.x,
                                   (y + 0.5f) / table_size * r.y);
            float val = filter->evaluate(p);
            func[i] = ocarina::abs(val);
            _lut.at(i) = val;
        }
        float sum = std::accumulate(_lut.begin(), _lut.end(), 0.f);
        float integral = sum / len;
        std::transform(_lut.cbegin(), _lut.cend(), _lut.begin(), [&](float val) {
            return val / integral;
        });
        _lut.reset_device_buffer_immediately(filter->device());
        _lut.upload_immediately();
        _lut.register_self();
        _warper->build(ocarina::move(func), make_uint2(table_size));
        _warper->prepare();
    }

    template<typename X, typename Y>
    requires is_all_integral_expr_v<X, Y>
    [[nodiscard]] Float lut_at(const X &x, const Y &y) const noexcept {
        Var index = y * table_size + x;
        return _lut.read(index);
    }

    template<typename V2>
    [[nodiscard]] Float lut_at(const V2 &v) const noexcept {
        return lut_at(v.x, v.y);
    }

    [[nodiscard]] FilterSample sample(Float2 u) const noexcept {
        u = u * 2.f - make_float2(1.f);
        Float pdf = 0.f;
        Uint2 offset;
        Float2 p = _warper->sample_continuous(u, std::addressof(pdf), std::addressof(offset));
        p = p * sign(u);
        return FilterSample{p, lut_at(offset) / pdf};
    }
};

class FittedCurveFilter : public Filter {
protected:
    FilterSampler _sampler{};

public:
    explicit FittedCurveFilter(const FilterDesc &desc)
        : Filter(desc) {}

    void prepare() noexcept override {
        _sampler.prepare(this);
    }

    [[nodiscard]] FilterSample sample(Float2 u) const noexcept override {
        FilterSample fs = _sampler.sample(u);
        fs.p = fs.p * radius();
        return fs;
    }
};

}// namespace vision