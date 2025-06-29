//
// Created by Zero on 11/10/2022.
//

#pragma once

#include "math/basic_types.h"
#include "rhi/common.h"
#include "dsl/dsl.h"
#include "base/node.h"
#include "tonemapper.h"
#include "math/box.h"

namespace vision {
using namespace ocarina;

//"radiance_collector": {
//    "type": "rgb",
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

protected:
    uint2 resolution_;
    Box2f screen_window_;
    EncodedData<uint> accumulation_;
    EncodedData<uint> gamma_correction_;
    TToneMapper tone_mapper_{};
    EncodedData<float> exposure_{};

public:
    RadianceCollector() = default;
    explicit RadianceCollector(const RadianceCollectorDesc &desc);
    OC_ENCODABLE_FUNC(Encodable, accumulation_,gamma_correction_, tone_mapper_, exposure_)
    VS_HOTFIX_MAKE_RESTORE(Node, resolution_, screen_window_, gamma_correction_,
                           accumulation_, tone_mapper_, exposure_)
    VS_MAKE_GUI_STATUS_FUNC(Node, tone_mapper_)
    virtual void compile() noexcept = 0;
    [[nodiscard]] uint pixel_num() const noexcept { return resolution_.x * resolution_.y; }
    [[nodiscard]] Box2f screen_window() const noexcept { return screen_window_; }
    [[nodiscard]] bool enable_accumulation() const noexcept { return accumulation_.hv(); }
    void set_resolution(uint2 res) noexcept { resolution_ = res; }
    void update_resolution(uint2 res) noexcept {
        set_resolution(res);
        on_resize(res);
        update_screen_window();
    }
    [[nodiscard]] virtual uint2 launch_dim() const noexcept { return resolution_; }
    [[nodiscard]] virtual uint frame_buffer_size() const noexcept {
        uint2 dim = launch_dim();
        return dim.x * dim.y;
    }
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    void update_screen_window() noexcept;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] Float4 apply_exposure(const Float4 &input) const noexcept ;
    virtual void on_resize(uint2 res) noexcept {}
    [[nodiscard]] auto tone_mapper() const noexcept { return tone_mapper_; }
    [[nodiscard]] auto tone_mapper() noexcept { return tone_mapper_; }
    [[nodiscard]] uint2 resolution() const noexcept { return resolution_; }
    virtual Float3 add_sample(const Uint2 &pixel, Float4 val, const Uint &frame_index) noexcept = 0;
    virtual Float3 add_sample(const Uint2 &pixel, const Float3 &val, const Uint &frame_index) noexcept {
        return add_sample(pixel, make_float4(val, 1.f), frame_index);
    }
    [[nodiscard]] virtual CommandList accumulate(BufferView<float4> input, BufferView<float4> output,
                                                 uint frame_index) const noexcept = 0;
    [[nodiscard]] virtual CommandList tone_mapping(BufferView<float4> input,
                                                   BufferView<float4> output) const noexcept = 0;
    [[nodiscard]] virtual const RegistrableManaged<float4> &output_buffer() const noexcept = 0;
    [[nodiscard]] virtual RegistrableManaged<float4> &output_buffer() noexcept = 0;
    [[nodiscard]] virtual const RegistrableManaged<float4> &accumulation_buffer() const noexcept = 0;
    [[nodiscard]] virtual RegistrableManaged<float4> &accumulation_buffer() noexcept = 0;
    [[nodiscard]] virtual const RegistrableManaged<float4> &rt_buffer() const noexcept = 0;
    [[nodiscard]] virtual RegistrableManaged<float4> &rt_buffer() noexcept = 0;
};
}// namespace vision