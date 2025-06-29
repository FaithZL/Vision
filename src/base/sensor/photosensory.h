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
#include "radiance_collector.h"
#include "hotfix/hotfix.h"

namespace vision {
struct SensorSample {
    Float2 p_film;
    Float2 p_lens;
    Float time;
    Float filter_weight{1.f};
    SensorSample() = default;
    explicit SensorSample(const Uint2 &pixel)
        : p_film(pixel + 0.5f) {}
};

class Photosensory : public Node, public EncodedObject, public Observer {
public:
    using Desc = SensorDesc;

protected:
    TFilter filter_{};
    HotfixSlot<SP<RadianceCollector>> rad_collector_{};
    EncodedData<uint> medium_id_{InvalidUI32};

public:
    Photosensory() = default;
    explicit Photosensory(const SensorDesc &desc);
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    OC_ENCODABLE_FUNC(EncodedObject, filter_, rad_collector_)
    VS_HOTFIX_MAKE_RESTORE(Node, filter_, rad_collector_, medium_id_, datas_)
    VS_MAKE_GUI_STATUS_FUNC(Node, filter_, rad_collector_)
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    void prepare() noexcept override;
    void upload_immediately() noexcept override {
        EncodedObject::upload_immediately();
    }
    [[nodiscard]] auto &filter() noexcept { return filter_; }
    [[nodiscard]] auto &filter() const noexcept { return filter_; }
    [[nodiscard]] auto rad_collector() noexcept { return rad_collector_.get(); }
    [[nodiscard]] auto rad_collector() const noexcept { return rad_collector_.get(); }
    [[nodiscard]] uint2 resolution() const noexcept { return rad_collector_->resolution(); }
    [[nodiscard]] uint frame_buffer_size() const noexcept { return rad_collector_->frame_buffer_size(); }
    virtual void set_resolution(uint2 res) noexcept { rad_collector_->set_resolution(res); }
    [[nodiscard]] virtual RayState generate_ray(const SensorSample &ss) const noexcept = 0;
};

}// namespace vision
