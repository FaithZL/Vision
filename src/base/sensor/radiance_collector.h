//
// Created by ling.zhu on 2025/6/27.
//

#pragma once

#include "base/node.h"
#include "base/sample.h"
#include "math/transform.h"
#include "filter.h"
#include "tonemapper.h"
#include "hotfix/hotfix.h"

namespace vision {
using namespace ocarina;

//"radiance_collector": {
//    "type": "normal",
//    "param": {
//        "resolution": [
//            1280,
//            720
//            ],
//         "fb_state": 0
//    }
//}
class RadianceCollector : public Node, public Encodable, public Observer {
public:
    using Desc = RadianceCollectorDesc;

private:
    uint2 resolution_;
    Box2f screen_window_;
    EncodedData<uint> accumulation_;
    EncodedData<uint> gamma_correction_;
    TToneMapper tone_mapper_{};
    EncodedData<float> exposure_{};

public:
    RadianceCollector() = default;
    explicit RadianceCollector(const RadianceCollectorDesc &desc)
        : Node(desc) {}
    OC_ENCODABLE_FUNC(Encodable, accumulation_, tone_mapper_, gamma_correction_, exposure_)
    VS_HOTFIX_MAKE_RESTORE(Node, resolution_, screen_window_, accumulation_,
                           tone_mapper_, gamma_correction_, exposure_)
    VS_MAKE_GUI_STATUS_FUNC(Node, tone_mapper_)
    [[nodiscard]] virtual uint2 launch_dim() const noexcept { return resolution_; }
    [[nodiscard]] virtual uint launch_num() const noexcept {
        uint2 dim = launch_dim();
        return dim.x * dim.y;
    }
    void set_resolution(uint2 res) noexcept { resolution_ = res; }
    void update_resolution(uint2 res) noexcept {
        set_resolution(res);
        on_resize(res);
        update_screen_window();
    }
    virtual void on_resize(uint2 res) noexcept {}
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    void update_screen_window() noexcept;
    [[nodiscard]] auto tone_mapper() const noexcept { return tone_mapper_; }
    [[nodiscard]] auto tone_mapper() noexcept { return tone_mapper_; }
    [[nodiscard]] uint pixel_num() const noexcept { return resolution_.x * resolution_.y; }
    virtual void add_sample(const Uint index, const Float3 &val, const Uint &frame_index) noexcept;
    [[nodiscard]] virtual const RegistrableManaged<float4> &rt_buffer() const noexcept = 0;
    [[nodiscard]] virtual RegistrableManaged<float4> &rt_buffer() noexcept = 0;

};

}// namespace vision