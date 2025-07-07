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
    prepare_view_buffer();
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

void FrameBuffer::compile_gamma() noexcept {
    Kernel kernel = [&](BufferVar<float4> input, BufferVar<float4> output) {
        Float4 val = input.read(dispatch_id());
        val = linear_to_srgb(val);
        val.w = 1.f;
        output.write(dispatch_id(), val);
    };
    gamma_correct_ = device().compile(kernel, "FrameBuffer-gamma_correction");
}

void FrameBuffer::compile_compute_geom() noexcept {
    TSensor &camera = scene().sensor();
    TSampler &sampler = scene().sampler();
    TLightSampler &light_sampler = scene().light_sampler();
    Kernel kernel = [&](Uint frame_index, BufferVar<PixelGeometry> gbuffer, BufferVar<float2> motion_vectors,
                        BufferVar<float4> albedo_buffer, BufferVar<float4> emission_buffer, BufferVar<float4> normal_buffer) {
        RenderEnv render_env;
        render_env.initial(sampler, frame_index, spectrum());
        Uint2 pixel = dispatch_idx().xy();
        sampler->start(pixel, frame_index, 0);
        camera->load_data();

        const SampledWavelengths &swl = render_env.sampled_wavelengths();

        SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);
        TriangleHitVar hit = pipeline()->trace_closest(rs.ray);

        Float2 motion_vec = make_float2(0.f);

        PixelGeometryVar geom;
        geom.p_film = ss.p_film;
        Float3 albedo;
        Float3 emission;
        $if(hit->is_hit()) {
            Interaction it = pipeline()->compute_surface_interaction(hit, rs.ray, true);
            geom->set_normal(it.shading.normal());
            normal_buffer.write(dispatch_id(), make_float4(it.shading.normal(), 1.f));
            geom.linear_depth = camera->linear_depth(it.pos);
            $if(it.has_material()) {
                scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                    MaterialEvaluator bsdf = material->create_evaluator(it, swl);
                    albedo = spectrum()->linear_srgb(bsdf.albedo(it.wo), swl);
                });
            };
            $if(it.has_emission()) {
                LightSampleContext p_ref;
                p_ref.pos = rs.origin();
                p_ref.ng = rs.direction();
                LightEval eval = light_sampler->evaluate_hit_wi(p_ref, it, swl);
                emission = spectrum()->linear_srgb(eval.L, swl);
            };
            motion_vec = compute_motion_vec(camera, ss.p_film, it.pos, true);
        };
        gbuffer.write(dispatch_id(), geom);
        motion_vectors.write(dispatch_id(), motion_vec);
        albedo_buffer.write(dispatch_id(), make_float4(albedo, 1.f));
        emission_buffer.write(dispatch_id(), make_float4(emission, 1.f));
    };
    compute_geom_ = device().compile(kernel, "rt_geom");
}

void FrameBuffer::compute_gradient(PixelGeometryVar &center_data,
                                   const BufferVar<PixelGeometry> &gbuffer) const noexcept {
    Uint x_sample_num = 0u;
    Uint y_sample_num = 0u;
    Float3 normal_dx = make_float3(0.f);
    Float3 normal_dy = make_float3(0.f);

    Float depth_dx = 0.f;
    Float depth_dy = 0.f;

    Uint2 center = dispatch_idx().xy();
    foreach_neighbor(dispatch_idx().xy(), [&](const Int2 &pixel) {
        Uint index = dispatch_id(pixel);
        PixelGeometryVar neighbor_data = gbuffer.read(index);
        $if(center.x > pixel.x) {
            x_sample_num += 1;
            normal_dx += center_data.normal_fwidth.xyz() - neighbor_data.normal_fwidth.xyz();
            depth_dx += center_data.linear_depth - neighbor_data.linear_depth;
        }
        $elif(pixel.x > center.x) {
            x_sample_num += 1;
            normal_dx += neighbor_data.normal_fwidth.xyz() - center_data.normal_fwidth.xyz();
            depth_dx += neighbor_data.linear_depth - center_data.linear_depth;
        };

        $if(center.y > pixel.y) {
            y_sample_num += 1;
            normal_dy += center_data.normal_fwidth.xyz() - neighbor_data.normal_fwidth.xyz();
            depth_dy += center_data.linear_depth - neighbor_data.linear_depth;
        }
        $elif(pixel.y > center.y) {
            y_sample_num += 1;
            normal_dy += neighbor_data.normal_fwidth.xyz() - center_data.normal_fwidth.xyz();
            depth_dy += neighbor_data.linear_depth - center_data.linear_depth;
        };
    });
    normal_dx /= cast<float>(x_sample_num);
    normal_dy /= cast<float>(y_sample_num);
    Float3 normal_fwidth = abs(normal_dx) + abs(normal_dy);
    center_data->set_normal_fwidth(length(normal_fwidth));

    depth_dx /= x_sample_num;
    depth_dy /= y_sample_num;
    center_data.depth_gradient = abs(depth_dx) + abs(depth_dy);
}

void FrameBuffer::compile_compute_grad() noexcept {
    Kernel kernel = [&](Uint frame_index, BufferVar<PixelGeometry> gbuffer) {
        PixelGeometryVar center_data = gbuffer.read(dispatch_id());
        compute_gradient(center_data, gbuffer);
        gbuffer.write(dispatch_id(), center_data);
    };
    compute_grad_ = device().compile(kernel, "rt_gradient");
}

void FrameBuffer::compile_compute_hit() noexcept {
    TSensor &camera = scene().sensor();
    TSampler &sampler = scene().sampler();
    Kernel kernel = [&](BufferVar<TriangleHit> hit_buffer, Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();
        sampler->start(pixel, frame_index, 0);
        camera->load_data();

        SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);
        TriangleHitVar hit = pipeline()->trace_closest(rs.ray);
        hit_buffer.write(dispatch_id(), hit);
    };
    compute_hit_ = device().compile(kernel, "rt_compute_pixel");
}

void FrameBuffer::compile() noexcept {
    compile_gamma();
    compile_compute_geom();
    compile_compute_grad();
    compile_compute_hit();
}

CommandList FrameBuffer::compute_hit(uint frame_index) const noexcept {
    CommandList ret;
    ret << compute_hit_(hit_buffer_, frame_index).dispatch(resolution());
    return ret;
}

CommandList FrameBuffer::compute_geom(uint frame_index, BufferView<PixelGeometry> gbuffer, BufferView<float2> motion_vectors,
                                      BufferView<float4> albedo, BufferView<float4> emission,BufferView<float4> normal) const noexcept {
    CommandList ret;
    ret << compute_geom_(frame_index, gbuffer, motion_vectors, albedo, emission, normal).dispatch(resolution());
    return ret;
}

CommandList FrameBuffer::compute_grad(uint frame_index, BufferView<vision::PixelGeometry> gbuffer) const noexcept {
    CommandList ret;
    ret << compute_grad_(frame_index, gbuffer).dispatch(resolution());
    return ret;
}

CommandList FrameBuffer::compute_GBuffer(uint frame_index, BufferView<PixelGeometry> gbuffer,
                                         BufferView<float2> motion_vectors,
                                         BufferView<float4> albedo,BufferView<float4> emission,
                                         BufferView<float4> normal) const noexcept {
    CommandList ret;
    ret << compute_geom_(frame_index, gbuffer, motion_vectors, albedo, emission, normal).dispatch(resolution());
    ret << compute_grad_(frame_index, gbuffer).dispatch(resolution());
    return ret;
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
    reset_surface_exts();
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