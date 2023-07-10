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

namespace detail {
[[nodiscard]] Bool is_valid(const Uint &tri_id) noexcept {
    return tri_id != InvalidUI32;
}
}// namespace detail

tuple<Float3, Float3, Bool> Baker::fetch_geometry_data(const BufferVar<Triangle> &triangles,
                                                       const BufferVar<Vertex> &vertices,
                                                       const BufferVar<uint4> &pixels) noexcept {
    Sampler *sampler = scene().sampler();
    Filter *filter = scene().camera()->filter();
    Uint4 pixel_data = pixels.read(dispatch_id());
    Uint triangle_id = pixel_data.x;
    Uint pixel_offset = pixel_data.y;
    Uint2 res = pixel_data.zw();
    Bool valid = detail::is_valid(triangle_id);
    Float3 norm;
    Float3 position;
    $if(valid) {
        Uint pixel_index = dispatch_id() - pixel_offset;

        Uint x = pixel_index % res.x;
        Uint y = pixel_index / res.x;

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

        Float2 u = sampler->next_2d();
        // todo Handle the case coord outside the triangle
        Float2 coord = make_float2(x + u.x, y + u.y);
        Float2 bary = barycentric(coord, p0, p1, p2);

        position = triangle_lerp(bary, v0->position(), v1->position(), v2->position());
        $if(is_zero(n0) || is_zero(n1) || is_zero(n2)) {
            Var v02 = v2->position() - v0->position();
            Var v01 = v1->position() - v0->position();
            norm = normalize(cross(v01, v02));
        }
        $else {
            norm = normalize(triangle_lerp(bary, n0, n1, n2));
        };
    };
    return {position, norm, valid};
}

void Baker::_compile_bake() noexcept {
    Sampler *sampler = scene().sampler();
    Integrator *integrator = scene().integrator();
    Kernel kernel = [&](Uint frame_index, BufferVar<Triangle> triangles,
                        BufferVar<Vertex> vertices, BufferVar<uint4> pixels,
                        BufferVar<float4> radiance) {
        sampler->start_pixel_sample(dispatch_idx().xy(), frame_index, 0);
        auto [position, norm, valid] = fetch_geometry_data(triangles, vertices, pixels);
        $if(!valid) {
            $return();
        };
        Float scatter_pdf;
        RayState rs = generate_ray(position, norm, &scatter_pdf);
        Interaction it;
        Float3 L = integrator->Li(rs, scatter_pdf, &it);
        Float4 result = make_float4(L, 1.f);
        result.w = select(dot(rs.direction(), it.ng) > 0, 0.f, 1.f);
        Float4 accum_prev = radiance.read(dispatch_id());
        Float a = 1.f / (frame_index + 1);
        result = lerp(make_float4(a), accum_prev, result);
        radiance.write(dispatch_id(), result);
    };

    _baker = device().compile(kernel, "baker");
}

void Baker::compile() noexcept {
    _compile_bake();
    _dilate_filter.compile();
    _batch_mesh.compile();
}

void Baker::_prepare(ocarina::span<BakedShape> baked_shapes) noexcept {
    VS_BAKER_STATS(_baker_stats, raster)
    _batch_mesh.setup(baked_shapes);
}

void Baker::_baking() noexcept {
    {
        VS_BAKER_STATS(_baker_stats, bake)
        Sampler *sampler = scene().sampler();
        for (uint i = 0; i < sampler->sample_per_pixel(); ++i) {
            _baker_stats.set_sample_index(i);
            stream() << _baker(i, _batch_mesh.triangles(),
                               _batch_mesh.vertices(),
                               _batch_mesh.pixels(),
                               _radiance)
                            .dispatch(_batch_mesh.pixel_num());
            stream() << synchronize();
            stream() << commit();
        }
        stream() << Printer::instance().retrieve() << synchronize();
        stream() << commit();
    }

    {
        VS_BAKER_STATS(_baker_stats, filter)
        stream() << _dilate_filter(_batch_mesh.pixels(),
                                   _radiance, _final_radiance)
                        .dispatch(_batch_mesh.pixel_num())
                 << synchronize() << commit();
    }
}

void Baker::_save_result(ocarina::span<BakedShape> baked_shapes) noexcept {
    VS_BAKER_STATS(_baker_stats, save)
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
    _baker_stats.on_batch_start(baked_shapes);
    _prepare(baked_shapes);
    _baking();
    _baker_stats.on_batch_end();
    _save_result(baked_shapes);
}

CommandList Baker::clear() noexcept {
    CommandList ret;
    ret << _radiance.clear();
    ret << _final_radiance.clear();
    ret << _batch_mesh.clear();
    ret << _batch_mesh.reset_pixels();
    return ret;
}

uint Baker::calculate_buffer_size() noexcept {
    return 2048 * 1024;
}

void Baker::allocate() noexcept {
    uint buffer_size = calculate_buffer_size();
    _radiance = device().create_buffer<float4>(buffer_size);
    _final_radiance = device().create_buffer<float4>(buffer_size);
    _batch_mesh.allocate(buffer_size);
}

CommandList Baker::deallocate() noexcept {
    CommandList ret;
    ret << _radiance.reallocate(0)
        << _final_radiance.reallocate(0);
    return ret;
}

}// namespace vision