//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "base/node.h"
#include "base/serial_object.h"
#include "base/sample.h"
#include "math/transform.h"
#include "base/scattering/medium.h"
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
    SP<Filter> _filter{};
    SP<Film> _radiance_film{};
    Wrap<Medium> _medium{};
    Serial<uint> _medium_id{InvalidUI32};

public:
    explicit Sensor(const SensorDesc &desc);
    OC_SERIALIZABLE_FUNC(SerialObject, *_filter, *_radiance_film)
    void prepare() noexcept override;
    [[nodiscard]] Filter *filter() noexcept { return _filter.get(); }
    [[nodiscard]] const Filter *filter() const noexcept { return _filter.get(); }
    [[nodiscard]] auto radiance_film() noexcept { return _radiance_film.get(); }
    [[nodiscard]] auto radiance_film() const noexcept { return _radiance_film.get(); }
    [[nodiscard]] uint2 resolution() noexcept { return _radiance_film->resolution(); }
    virtual void set_resolution(uint2 res) noexcept { _radiance_film->set_resolution(res); }
    [[nodiscard]] virtual RayState generate_ray(const SensorSample &ss) const noexcept = 0;
};

}// namespace vision
