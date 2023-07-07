//
// Created by Zero on 2023/7/3.
//

#include "baker.h"

namespace vision {

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
    Kernel k = [&](Uint frame_index, BufferVar<float4> positions,
                   BufferVar<float4> normals, BufferVar<float4> radiance) {
        Uint pixel_index = dispatch_id();
        Float4 position = positions.read(pixel_index);
        Float4 normal = normals.read(pixel_index);
        sampler->start_pixel_sample(dispatch_idx().xy(), frame_index, 0);
        $if(!detail::is_valid(normals.read(dispatch_id()))) {
            $return();
        };
        Float scatter_pdf;
        RayState rs = generate_ray(position.xyz(), normal.xyz(), &scatter_pdf);
        Interaction it;
        Float3 L = integrator->Li(rs, scatter_pdf, &it);
        Float4 result = make_float4(L, 1.f);
        result.w = select(dot(rs.direction(), it.g_uvn.normal()) > 0, 0.f, 1.f);
        Float4 accum_prev = radiance.read(dispatch_id());
        Float a = 1.f / (frame_index + 1);
        result = lerp(make_float4(a), accum_prev, result);
        radiance.write(dispatch_id(), result);
    };

    _bake_shader = device().compile(k, "baker old");

    Kernel kernel = [&](Uint frame_index, BufferVar<Triangle> triangles,
                        BufferVar<Vertex> vertices, BufferVar<uint4> pixels,
                        BufferVar<float4> radiance) {
        Uint4 pixel_data = pixels.read(dispatch_id());
        Uint triangle_id = pixel_data.x;
        Uint pixel_offset = pixel_data.y;
        Uint2 res = pixel_data.zw();
        auto is_valid = [&](const Uint &tri_id) {
            return tri_id != InvalidUI32;
        };
        $if(!is_valid(triangle_id)) {
            $return();
        };
        Uint pixel_index = dispatch_id() - pixel_offset;

        Uint x = pixel_index % res.x;
        Uint y = pixel_index / res.x;

        Float2 coord = make_float2(x + 0.5f, y + 0.5f);
        Var tri = triangles.read(triangle_id);
        Var v0 = vertices.read(tri.i);
        Var v1 = vertices.read(tri.j);
        Var v2 = vertices.read(tri.k);

        Float2 p0 = v0->lightmap_uv();
        Float2 p1 = v1->lightmap_uv();
        Float2 p2 = v2->lightmap_uv();
        Float3 n0 = v0->normal();
        Float3 n1 = v1->normal();
        Float3 n2 = v2->normal();

        Float2 bary = barycentric(coord, p0, p1, p2);
        Float3 norm;
        Float3 position = triangle_lerp(bary, v0->position(), v1->position(), v2->position());
        $if(is_zero(n0) || is_zero(n1) || is_zero(n2)) {
            Var v02 = v2->position() - v0->position();
            Var v01 = v1->position() - v0->position();
            norm = normalize(cross(v01, v02));
        } $else {
            norm = normalize(triangle_lerp(bary, n0, n1, n2));
        };
        radiance.write(dispatch_id(), make_float4(position, 1.f));
    };

    _baker = device().compile(kernel, "baker");
}

void Baker::_compile_transform() noexcept {
    Kernel kernel = [&](BufferVar<float4> positions,
                        BufferVar<float4> normals, Float4x4 o2w,
                        Uint offset, Uint2 res, Float surface_ofs) {
        Float4 position = positions.read(dispatch_id());
        Float4 normal = normals.read(dispatch_id());
        Float3 world_pos = make_float3(0);
        Float3 world_norm = make_float3(0);
        $if(position.w > 0.f) {
            world_pos = transform_point(o2w, position.xyz());
            world_norm = normalize(transform_normal(o2w, normal.xyz()));
            world_pos += world_norm * surface_ofs;
        };
        positions.write(dispatch_id(), make_float4(world_pos, as<float>(offset)));
        normals.write(dispatch_id(), make_float4(world_norm, detail::uint2_to_float(res)));
    };
    _transform_shader = device().compile(kernel, "transform shader");
}

void Baker::compile() noexcept {
    _rasterizer->compile_shader();
    _compile_bake();
    _compile_transform();
    _dilate_filter.compile();
    _batch_mesh.compile();
}

void Baker::_prepare(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint offset = 0;

    _batch_mesh.setup(baked_shapes, calculate_buffer_size());

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
                                 _normals, _final_radiance)
                        .dispatch(pixel_num());
    }

//    for (uint i = 0; i < sampler->sample_per_pixel(); ++i) {
//        stream() << _baker(i, _batch_mesh.triangles(),
//                           _batch_mesh.vertices(),
//                           _batch_mesh.pixels(),
//                           _final_radiance).dispatch(pixel_num());
//        stream() << Printer::instance().retrieve();
//    }

    //    stream() << Printer::instance().retrieve();
//    stream() << _dilate_filter(_positions, _normals,
//                               _radiance, _final_radiance)
//                    .dispatch(pixel_num())
//             << Printer::instance().retrieve()
//             << synchronize() << commit();
}

void Baker::_save_result(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint offset = 0;
    for (BakedShape &bs : baked_shapes) {
        Context::create_directory_if_necessary(bs.instance_cache_directory());
        bs.allocate_lightmap_texture();
        stream() << bs.lightmap_tex().copy_from(_final_radiance, offset);
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
    _final_radiance = device().create_buffer<float4>(buffer_size);
    _batch_mesh.init(buffer_size);
}

CommandList Baker::deallocate() noexcept {
    CommandList ret;
    ret << _positions.reallocate(0)
        << _normals.reallocate(0)
        << [&] { _pixel_num.clear(); }
        << _radiance.reallocate(0)
        << _final_radiance.reallocate(0);
    return ret;
}

}// namespace vision