//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "math/basic_types.h"
#include "base/node.h"
#include "base/encoded_object.h"
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
    SensorSample() = default;
    explicit SensorSample(const Uint2 &pixel)
        : p_film(pixel + 0.5f) {}
};

class Sensor : public Node, public EncodedObject {
public:
    using Desc = SensorDesc;

protected:
    Filter filter_{};
    SP<Film> film_{};
    TObject<Medium> medium_{};
    EncodedData<uint> medium_id_{InvalidUI32};

public:
    explicit Sensor(const SensorDesc &desc);
    OC_ENCODABLE_FUNC(EncodedObject, filter_, film_)
    VS_MAKE_GUI_STATUS_FUNC(Node, filter_, film_)
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void prepare() noexcept override;
    [[nodiscard]] auto& filter() noexcept { return filter_; }
    [[nodiscard]] auto& filter() const noexcept { return filter_; }
    [[nodiscard]] auto film() noexcept { return film_.get(); }
    [[nodiscard]] auto film() const noexcept { return film_.get(); }
    [[nodiscard]] uint2 resolution() const noexcept { return film_->resolution(); }
    virtual void set_resolution(uint2 res) noexcept { film_->set_resolution(res); }
    [[nodiscard]] virtual RayState generate_ray(const SensorSample &ss) const noexcept = 0;
};

using TSensor = TObject<Sensor>;

}// namespace vision
