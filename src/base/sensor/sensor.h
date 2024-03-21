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
    SP<Film> _film{};
    Wrap<Medium> _medium{};
    Serial<uint> _medium_id{InvalidUI32};

public:
    explicit Sensor(const SensorDesc &desc);
    OC_SERIALIZABLE_FUNC(SerialObject, *_filter, *_film)
    VS_MAKE_GUI_STATUS_FUNC(Node, _filter, _film)
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void prepare() noexcept override;
    [[nodiscard]] Filter *filter() noexcept { return _filter.get(); }
    [[nodiscard]] const Filter *filter() const noexcept { return _filter.get(); }
    [[nodiscard]] auto film() noexcept { return _film.get(); }
    [[nodiscard]] auto film() const noexcept { return _film.get(); }
    [[nodiscard]] uint2 resolution() noexcept { return _film->resolution(); }
    virtual void set_resolution(uint2 res) noexcept { _film->set_resolution(res); }
    [[nodiscard]] virtual RayState generate_ray(const SensorSample &ss) const noexcept = 0;
};

}// namespace vision
