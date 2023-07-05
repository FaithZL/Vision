//
// Created by Zero on 2023/7/3.
//

#include "baker.h"

namespace vision {

namespace detail {
[[nodiscard]] Uint encode(Uint a, Uint b) noexcept {
    a = a << 16;
    return a | b;
}

[[nodiscard]] Uint2 decode(Uint arg) noexcept {
    Uint a = (0xffff0000 & arg) >> 16;
    Uint b = 0x0000ffff & arg;
    return make_uint2(a, b);
}
}// namespace detail

RayState Baker::generate_ray(const Float3 &position, const Float3 &normal, Float *scatter_pdf) const noexcept {
    Sampler *sampler = scene().sampler();
    Float3 wi = square_to_cosine_hemisphere(sampler->next_2d());
    *scatter_pdf = cosine_hemisphere_PDF(wi.z);
    Frame frame(normal);
    OCRay ray = vision::spawn_ray(position, normal, frame.to_world(wi));
    return {.ray = ray, .ior = 1.f, .medium = InvalidUI32};
}

void Baker::_compile_bake() noexcept {
    Sampler *sampler = scene().sampler();
    Integrator *integrator = scene().integrator();

    auto jitter = [&](const BufferVar<float4> &positions,
                      const BufferVar<float4> &normals,
                      const BufferVar<float4> &radiance,
                      Uint2 *pixel,
                      Uint2 *res,
                      Float3 *norm,
                      Float3 *pos) {
        Uint pixel_index = dispatch_id();
        Float4 position = positions.read(pixel_index);
        Float4 normal = normals.read(pixel_index);
        Uint offset = as<uint>(position.w);
        Uint cur_index = pixel_index - offset;
        *res = detail::decode(as<uint>(normal.w));
        *pixel = make_uint2(cur_index % res->x, cur_index / res->x);

        auto in_bound = [&](const Int2 &p) -> Bool {
            return all(p >= 0) && all(p < make_int2(*res));
        };

        auto is_valid = [&](Uint g_index) -> Bool {
            Float4 p_normal = normals.read(g_index);
            Uint2 p_res = detail::decode(as<uint>(p_normal.w));
            return p_res.x > 0;
        };
        Uint u_num = 0;
        Uint v_num = 0;
        Float3 cur_pos = position.xyz();
        Float3 cur_norm = normal.xyz();
        Float3 abs_u = make_float3(0.f);
        Float3 abs_v = make_float3(0.f);
        $if(res->x > 0) {
            $for(x, -1, 2) {
                $for(y, -1, 2) {
                    Int2 p = make_int2(*pixel) + make_int2(x, y);
                    Uint p_index = p.y * res->x + p.x;
                    Uint g_index = offset + p_index;
                    $if(!in_bound(p)) {
                        $continue;
                    };
                    $if(is_valid(g_index) && (x != 0 || y != 0)) {
                        Float3 p_pos = positions.read(g_index).xyz();
                        $if(x == 0) {
                            abs_v += abs(cur_pos - p_pos);
                            v_num += 1;
                        }
                        $elif(y == 0) {
                            abs_u += abs(cur_pos - p_pos);
                            u_num += 1;
                        }
                        $else{

                        };
                    };
                };
            };
            Float3 su = abs_u / cast<float>(u_num);
            Float3 sv = abs_v / cast<float>(v_num);

            Float3 lb = cur_pos - 0.5f * (su + sv);
            Float2 u2 = sampler->next_2d();
            *pos = lb + u2.x * su + u2.y * sv;
            $if(has_nan(su) || has_nan(sv)) {
                *pos = position.xyz();
            };
            *norm = cur_norm;
        };
    };

    Kernel kernel = [&](Uint frame_index, BufferVar<float4> positions,
                        BufferVar<float4> normals, BufferVar<float4> radiance) {
        Uint2 pixel;
        Uint2 res;
        Float3 position;
        Float3 normal;
        sampler->start_pixel_sample(dispatch_idx().xy(), frame_index, 0);
        jitter(positions, normals, radiance, &pixel, &res, &normal, &position);
        $if(res.x != 0u) {
            Float scatter_pdf;
            RayState rs = generate_ray(position, normal, &scatter_pdf);
            Interaction it;
            Float3 L = integrator->Li(rs, scatter_pdf, &it);
            Float4 result = make_float4(L, 1.f);
            result.w = select(dot(rs.direction(), it.g_uvn.normal()) > 0, 0.f, 1.f);
            Float4 accum_prev = radiance.read(dispatch_id());
            Float a = 1.f / (frame_index + 1);
            result = lerp(make_float4(a), accum_prev, result);
            radiance.write(dispatch_id(), result);
        };
    };
    _bake_shader = device().compile(kernel, "baker");
}

void Baker::_compile_transform() noexcept {
    Kernel kernel = [&](BufferVar<float4> positions,
                        BufferVar<float4> normals, Float4x4 o2w,
                        Uint offset, Uint2 res, Float surface_ofs) {
        Float4 position = positions.read(dispatch_id());
        Float4 normal = normals.read(dispatch_id());
        $if(position.w > 0.f) {
            Float3 world_pos = transform_point(o2w, position.xyz());
            Float3 world_norm = normalize(transform_normal(o2w, normal.xyz()));
            positions.write(dispatch_id(), make_float4(world_pos + world_norm * surface_ofs, as<float>(offset)));
            normals.write(dispatch_id(), make_float4(world_norm, as<float>(detail::encode(res.x, res.y))));
        };
    };
    _transform_shader = device().compile(kernel, "transform shader");
}

void Baker::compile() noexcept {
    _rasterizer->compile_shader();
    _compile_bake();
    _compile_transform();
}

void Baker::_prepare(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint offset = 0;
    for (BakedShape &bs : baked_shapes) {
        stream() << bs.prepare_for_rasterize();
        if (bs.has_rasterization_cache()) {
            stream() << bs.load_rasterization_from_cache();
        } else {
            stream() << _rasterizer->apply(bs) << bs.save_rasterization_to_cache();
        }
        stream() << [&] { bs.normalize_lightmap_uv(); };
        stream() << _transform_shader(bs.positions(), bs.normals(),
                                      bs.shape()->o2w(), offset,
                                      bs.resolution(), bs.surface_offset())
                        .dispatch(bs.resolution());
        stream() << append_buffer(bs.normals(), bs.positions());
        stream() << synchronize() << Printer::instance().retrieve() << commit();
        offset += bs.pixel_num();
    }
}

void Baker::_baking() noexcept {
    Sampler *sampler = scene().sampler();
    for (uint i = 0; i < sampler->sample_per_pixel(); ++i) {
        stream() << _bake_shader(i, _positions,
                                 _normals, _radiance)
                        .dispatch(pixel_num());
    }
    stream() << Printer::instance().retrieve() << synchronize() << commit();
}

void Baker::_save_result(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint offset = 0;
    for (BakedShape &bs : baked_shapes) {
        Context::create_directory_if_necessary(bs.instance_cache_directory());
        bs.allocate_lightmap_texture();
        stream() << bs.lightmap_tex().copy_from(_radiance, offset);
        stream() << bs.save_lightmap_to_cache();
        stream() << synchronize() << commit();
        pipeline()->register_texture(bs.lightmap_tex());
        offset += bs.pixel_num();
    }
}

void Baker::baking(ocarina::span<BakedShape> baked_shapes) noexcept {
    stream() << clear() << synchronize() << commit();
    _prepare(baked_shapes);
    _baking();
    _save_result(baked_shapes);
}

CommandList Baker::clear() noexcept {
    CommandList ret;
    ret << _positions.clear();
    ret << _normals.clear();
    ret << _radiance.clear();
    ret << [&] { _pixel_num.clear(); };
    return ret;
}

uint Baker::pixel_num() const noexcept {
    return std::accumulate(_pixel_num.begin(), _pixel_num.end(), 0u);
}

CommandList Baker::append_buffer(const Buffer<ocarina::float4> &normals,
                                 const Buffer<ocarina::float4> &positions) noexcept {
    CommandList ret;
    ret << _positions.copy_from(positions, pixel_num());
    ret << _normals.copy_from(normals, pixel_num());
    ret << [size = normals.size(), this] { _pixel_num.push_back(size); };
    return ret;
}

uint Baker::calculate_buffer_size() noexcept {
    return 2048 * 1024;
}

void Baker::allocate() noexcept {
    uint buffer_size = calculate_buffer_size();
    _positions = device().create_buffer<float4>(buffer_size);
    _normals = device().create_buffer<float4>(buffer_size);
    _radiance = device().create_buffer<float4>(buffer_size);
}

CommandList Baker::deallocate() noexcept {
    CommandList ret;
    ret << _positions.reallocate(0)
        << _normals.reallocate(0)
        << [&] { _pixel_num.clear(); }
        << _radiance.reallocate(0);
    return ret;
}

}// namespace vision