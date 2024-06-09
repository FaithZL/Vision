//
// Created by Zero on 2024/2/18.
//

#include "frame_buffer.h"
#include "mgr/pipeline.h"

namespace vision {
FrameBuffer::FrameBuffer(const vision::FrameBufferDesc &desc)
    : Node(desc) {}

bool FrameBuffer::render_UI(ocarina::Widgets *widgets) noexcept {
    return widgets->use_folding_header(
        ocarina::format("{} FrameBuffer", impl_type().data()),
        [&] {
            render_sub_UI(widgets);
        });
}

void FrameBuffer::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    auto show_buffer = [&](Managed<float4> &buffer) {
        if (buffer.device_buffer().size() == 0) {
            return;
        }

        widgets->use_tree(buffer.name(), [&] {
            buffer.download_immediately();
            ImageView image_view(buffer.host_buffer().data(), resolution());
            widgets->image(image_view);
        });
    };
}

void FrameBuffer::init_screen_buffer(ScreenBuffer &buffer) noexcept {
    buffer.reset_all(device(), pixel_num(), buffer.name());
    vector<float4> vec{};
    vec.assign(pixel_num(), float4{});
    buffer.set_bindless_array(bindless_array());
    buffer.register_self();
}

void FrameBuffer::register_(const SP<ScreenBuffer> &buffer) noexcept {
    screen_buffers_.insert(std::make_pair(buffer->name(), buffer));
}

void FrameBuffer::unregister(const SP<ScreenBuffer> &buffer) noexcept {
    //    buffer.unregister();
}

uint FrameBuffer::pixel_index(uint2 pos) const noexcept {
    return pos.y * resolution().x + pos.x;
}

uint FrameBuffer::pixel_num() const noexcept {
    return pipeline()->pixel_num();
}

uint2 FrameBuffer::resolution() const noexcept {
    return pipeline()->resolution();
}

BindlessArray &FrameBuffer::bindless_array() noexcept {
    return pipeline()->bindless_array();
}

BufferView<PixelGeometry> FrameBuffer::prev_gbuffer(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<PixelGeometry>(prev_gbuffer_index(frame_index));
}

BufferView<PixelGeometry> FrameBuffer::cur_gbuffer(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<PixelGeometry>(cur_gbuffer_index(frame_index));
}

Float2 FrameBuffer::compute_motion_vec(const Camera &camera, const Float2 &p_film,
                                       const Float3 &cur_pos, const Bool &is_hit) noexcept {
    Float2 ret = make_float2(0.f);
    $if(is_hit) {
        Float2 raster_coord = camera->prev_raster_coord(cur_pos).xy();
        ret = p_film - raster_coord;
    };
    return ret;
}

}// namespace vision