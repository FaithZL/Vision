//
// Created by Zero on 2024/2/18.
//

#include "frame_buffer.h"
#include "mgr/pipeline.h"

namespace vision {

void ScreenBuffer::update_resolution(ocarina::uint2 res, Device &device) noexcept {
    super().reset_all(device, res.x * res.y, name_);
    register_self();
}

FrameBuffer::FrameBuffer(const vision::FrameBufferDesc &desc)
    : Node(desc) {
    visualizer_->init();
}

void FrameBuffer::prepare() noexcept {
    init_buffer(view_buffer_, "FrameBuffer::view_buffer_");
}

void FrameBuffer::update_runtime_object(const vision::IObjectConstructor *constructor) noexcept {
    std::tuple tp = {addressof(visualizer_)};
    HotfixSystem::replace_objects(constructor, tp);
}

bool FrameBuffer::render_UI(ocarina::Widgets *widgets) noexcept {
    bool ret = widgets->use_folding_header(
        ocarina::format("{} FrameBuffer", impl_type().data()),
        [&] {
            render_sub_UI(widgets);
        });
    ret |= visualizer_->render_UI(widgets);
    return ret;
}

void FrameBuffer::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    auto show_buffer = [&](Managed<float4> &buffer) {
        if (buffer.device_buffer().size() == 0) {
            return;
        }
        if (widgets->radio_button(buffer.name(), cur_view_ == buffer.name())) {
            cur_view_ = buffer.name();
        }
    };
    for (auto iter = screen_buffers_.begin();
         iter != screen_buffers_.end(); ++iter) {
        show_buffer(*iter->second);
    }
}

void FrameBuffer::init_screen_buffer(const SP<ScreenBuffer> &buffer) noexcept {
    buffer->reset_all(device(), pixel_num(), buffer->name());
    vector<float4> vec{};
    vec.assign(pixel_num(), float4{});
    buffer->set_bindless_array(bindless_array());
    buffer->register_self();
}

void FrameBuffer::compile() noexcept {
    Kernel kernel = [&](BufferVar<float4> input, BufferVar<float4> output) {
        Float4 val = input.read(dispatch_id());
        val = linear_to_srgb(val);
        val.w = 1.f;
        output.write(dispatch_id(), val);
    };
    gamma_correct_ = device().compile(kernel, "FrameBuffer-gamma_correction");
}

CommandList FrameBuffer::gamma_correct(BufferView<float4> input,
                                       BufferView<float4> output) const noexcept {
    CommandList ret;
    ret << gamma_correct_(input,
                          output)
               .dispatch(resolution());
    return ret;
}

CommandList FrameBuffer::gamma_correct() const noexcept {
    const Buffer<float4> &input = cur_screen_buffer();
    return gamma_correct(input, view_buffer_);
}

void FrameBuffer::register_(const SP<ScreenBuffer> &buffer) noexcept {
    auto iter = screen_buffers_.find(buffer->name());
    if (iter != screen_buffers_.end()) {
        OC_ERROR("");
    }
    screen_buffers_.insert(std::make_pair(buffer->name(), buffer));
}

void FrameBuffer::unregister(const SP<ScreenBuffer> &buffer) noexcept {
    unregister(buffer->name());
}

void FrameBuffer::unregister(const std::string &name) noexcept {
    auto iter = screen_buffers_.find(name);
    if (iter == screen_buffers_.end()) {
        OC_ERROR("");
    }
    screen_buffers_.erase(iter);
}

uint FrameBuffer::pixel_index(uint2 pos) const noexcept {
    return pos.y * resolution().x + pos.x;
}

void FrameBuffer::fill_window_buffer(const Buffer<ocarina::float4> &input) noexcept {
    input.download_immediately(window_buffer_.data());
    visualizer_->draw(window_buffer_.data());
}

void FrameBuffer::resize(ocarina::uint2 res) noexcept {
    window_buffer_.resize(res.x * res.y, make_float4(0.f));
}

void FrameBuffer::update_resolution(ocarina::uint2 res) noexcept {
    resize(res);
    reset_surfaces();
    reset_surface_extends();
    reset_gbuffer();
    reset_hit_bsdfs();
    reset_motion_vectors();
    reset_hit_buffer();
    reset_buffer(view_buffer_, "FrameBuffer::view_buffer_");
    for (auto &it : screen_buffers_) {
        it.second->update_resolution(res, device());
    }
    pipeline()->upload_bindless_array();
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

void FrameBuffer::after_render() noexcept {
    fill_window_buffer(view_buffer_);
}

const Buffer<float4> &FrameBuffer::cur_screen_buffer() const noexcept {
    return screen_buffers_.at(cur_view_)->device_buffer();
}

BufferView<PixelGeometry> FrameBuffer::prev_gbuffer(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<PixelGeometry>(prev_gbuffer_index(frame_index));
}

BufferView<PixelGeometry> FrameBuffer::cur_gbuffer(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<PixelGeometry>(cur_gbuffer_index(frame_index));
}

BufferView<SurfaceData> FrameBuffer::prev_surfaces(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<SurfaceData>(prev_surfaces_index(frame_index));
}

BufferView<SurfaceData> FrameBuffer::cur_surfaces(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<SurfaceData>(cur_surfaces_index(frame_index));
}

BindlessArrayBuffer<SurfaceData> FrameBuffer::prev_surfaces(const ocarina::Uint &frame_index) const noexcept {
    return pipeline()->buffer_var<SurfaceData>(prev_surfaces_index(frame_index));
}

BindlessArrayBuffer<SurfaceData> FrameBuffer::cur_surfaces(const ocarina::Uint &frame_index) const noexcept {
    return pipeline()->buffer_var<SurfaceData>(cur_surfaces_index(frame_index));
}

Uint FrameBuffer::checkerboard_value(const Uint2 &coord) noexcept {
    return (coord.x & 1) ^ (coord.y & 1);
}

Uint FrameBuffer::checkerboard_value(const Uint2 &coord, const Uint &frame_index) noexcept {
    return checkerboard_value(coord) ^ (frame_index & 1);
}

Float2 FrameBuffer::compute_motion_vec(const TSensor &camera, const Float2 &p_film,
                                       const Float3 &cur_pos, const Bool &is_hit) noexcept {
    Float2 ret = make_float2(0.f);
    $if(is_hit) {
        Float2 raster_coord = camera->prev_raster_coord(cur_pos).xy();
        ret = p_film - raster_coord;
    };
    return ret;
}

Float3 FrameBuffer::compute_motion_vector(const TSensor &camera, const Float2 &p_film,
                                          const Uint &frame_index) const noexcept {
    Uint2 pixel = make_uint2(p_film);
    Uint pixel_index = dispatch_id(pixel);
    SurfaceDataVar cur_surf = cur_surfaces(frame_index).read(pixel_index);
    SurfaceDataVar prev_surf = prev_surfaces(frame_index).read(pixel_index);
    return compute_motion_vector(camera, cur_surf->position(), prev_surf->position());
}

Float3 FrameBuffer::compute_motion_vector(const TSensor &camera, const Float3 &cur_pos,
                                          const Float3 &pre_pos) const noexcept {
    Float3 cur_coord = camera->raster_coord(cur_pos);
    Float3 prev_coord = camera->raster_coord(pre_pos);
    return prev_coord - cur_coord;
}
}// namespace vision