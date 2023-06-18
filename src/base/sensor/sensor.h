//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "base/node.h"
#include "base/serial_object.h"
#include "base/sample.h"
#include "math/transform.h"
#include "filter.h"
#include "film.h"

namespace vision {
using namespace ocarina;

struct SensorSample {
    Float2 p_film;
    Float2 p_lens;
    Float time;
    Float filter_weight{1.f};
};

class Sensor : public Node, public SerialObject {
public:
    using Desc = SensorDesc;

protected:
    Filter *_filter{};
    Film *_radiance_film{};
    uint _medium{InvalidUI32};

public:
    explicit Sensor(const SensorDesc &desc);
    OC_SERIALIZABLE_FUNC(Serializable<float>, *_filter, *_radiance_film)
    void prepare() noexcept override;
    [[nodiscard]] Filter *filter() noexcept { return _filter; }
    [[nodiscard]] const Filter *filter() const noexcept { return _filter; }
    [[nodiscard]] auto radiance_film() noexcept { return _radiance_film; }
    [[nodiscard]] auto radiance_film() const noexcept { return _radiance_film; }
    [[nodiscard]] uint2 resolution() noexcept { return _radiance_film->resolution(); }
    [[nodiscard]] virtual RayState generate_ray(const SensorSample &ss) const noexcept = 0;
};

}// namespace vision

