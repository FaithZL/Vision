//
// Created by Zero on 2023/7/3.
//

#include "baker.h"

namespace vision {

RayState Baker::generate_ray(const Float3 &position, const Float3 &normal, Float *scatter_pdf) const noexcept {
    TSampler &sampler = scene().sampler();
    Float3 wi = square_to_cosine_hemisphere(sampler->next_2d());
    *scatter_pdf = cosine_hemisphere_PDF(wi.z);
    Frame frame(normal);
    RayVar ray = vision::spawn_ray(position, normal, frame.to_world(wi));
    return {.ray = ray, .ior = 1.f, .medium = InvalidUI32};
}

tuple<Float3, Float3, Bool, Float> Baker::fetch_geometry_data(const BufferVar<Triangle> &triangles,
                                                              const BufferVar<Vertex> &vertices,
                                                              const BufferVar<uint4> &pixels,
                                                              Float2 *p_film) noexcept {
    TSampler &sampler = scene().sampler();
    auto filter = scene().sensor()->filter();
    Uint4 pixel_data = pixels.read(dispatch_id());
    Uint triangle_id = pixel_data.x;
    Uint pixel_offset = pixel_data.y;
    Uint2 res = detail::uint_to_uint2(pixel_data.z);
    Bool valid = bit_cast<uint>(1.f) == pixel_data.w;
    Float3 norm;
    Float3 position;

    auto get_verts = [&](const Uint &triangle_id) -> ocarina::array<Var<Vertex>, 3> {
        Var tri = triangles.read(triangle_id);
        Var v0 = vertices.read(tri.i);
        Var v1 = vertices.read(tri.j);
        Var v2 = vertices.read(tri.k);
        return {v0, v1, v2};
    };
    Float weight = 1.f;
    $if(valid) {
        Uint pixel_index = dispatch_id() - pixel_offset;

        Uint x = pixel_index % res.x;
        Uint y = pixel_index / res.x;
        auto [v0, v1, v2] = get_verts(triangle_id);
        Float2 p0 = v0->lightmap_uv();
        Float2 p1 = v1->lightmap_uv();
        Float2 p2 = v2->lightmap_uv();
        auto ss = sampler->sensor_sample(make_uint2(x, y), filter);

        Float2 coord = ss.p_film;
        if (p_film) {
            *p_film = ss.p_film;
        }
        weight = ss.filter_weight;
        $if(!in_triangle<D>(coord, p0, p1, p2)) {
            coord = make_float2(x + 0.5f, y + 0.5f);
            weight = 1.f;
        };
        // todo Handle the case coord outside the triangle

        Float2 bary = barycentric(coord, p0, p1, p2);
        Float3 n0 = v0->normal();
        Float3 n1 = v1->normal();
        Float3 n2 = v2->normal();
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
    return {position, norm, valid, weight};
}

void Baker::_compile_bake() noexcept {
    TSampler &sampler = scene().sampler();
    TSensor camera = scene().sensor();
    TIntegrator &integrator = scene().integrator();
    Kernel kernel = [&](Uint frame_index, BufferVar<Triangle> triangles,
                        BufferVar<Vertex> vertices, BufferVar<uint4> pixels,
                        BufferVar<float4> radiance) {
        camera->load_data();
        sampler->load_data();
        integrator->load_data();
        RenderEnv render_env;
        render_env.initial(sampler, frame_index, spectrum());
        sampler->start(dispatch_idx().xy(), frame_index, 0);
        Float2 p_film = make_float2(dispatch_idx().xy()) + 0.5f;
        auto [position, norm, valid, weight] = fetch_geometry_data(triangles, vertices, pixels, &p_film);
        $if(!valid) {
            $return();
        };
        Float scatter_pdf;
        RayState rs = generate_ray(position, norm, &scatter_pdf);
        Interaction it{false};
        Float3 L = integrator->Li(rs, scatter_pdf, scene().spectrum()->one(), it, render_env) * weight;
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
    rasterizer_->compile();
    dilate_filter_.compile();
    batch_mesh_.compile();
}

void Baker::_prepare(ocarina::span<BakedShape> baked_shapes) noexcept {
    VS_BAKER_STATS(baker_stats_, raster)
    for (BakedShape &bs : baked_shapes) {
        bs.prepare_to_rasterize();
        rasterizer_->apply(bs);
        stream() << bs.save_rasterize_map_to_cache() << synchronize() << commit();
    }
    batch_mesh_.batch(baked_shapes);
}

void Baker::_baking() noexcept {
    {
        VS_BAKER_STATS(baker_stats_, bake)
        TSampler &sampler = scene().sampler();
        for (uint i = 0; i < sampler->sample_per_pixel(); ++i) {
            baker_stats_.set_sample_index(i);
            stream() << _baker(i, batch_mesh_.triangles(),
                               batch_mesh_.vertices(),
                               batch_mesh_.pixels(),
                               radiance_)
                            .dispatch(batch_mesh_.pixel_num());
            stream() << synchronize();
            stream() << commit();
        }
        stream() << Env::printer().retrieve() << synchronize();
        stream() << commit();
    }

    {
        VS_BAKER_STATS(baker_stats_, filter)
        stream() << dilate_filter_(batch_mesh_.pixels(),
                                   radiance_, final_radiance_)
                        .dispatch(batch_mesh_.pixel_num())
                 << synchronize() << commit();
    }
}

void Baker::_save_result(ocarina::span<BakedShape> baked_shapes) noexcept {
    VS_BAKER_STATS(baker_stats_, save)
    uint offset = 0;
    for (BakedShape &bs : baked_shapes) {
        bs.allocate_lightmap_texture();
        stream() << bs.lightmap_tex().copy_from(final_radiance_, offset);
        stream() << bs.save_lightmap_to_cache();
        stream() << synchronize() << commit();
        pipeline()->register_texture(bs.lightmap_tex());
        offset += bs.pixel_num();
    }
}

void Baker::baking(ocarina::span<BakedShape> baked_shapes) noexcept {
    stream() << clear() << synchronize() << commit();
    baker_stats_.on_batch_start(baked_shapes);
    _prepare(baked_shapes);
    _baking();
    baker_stats_.on_batch_end();
    _save_result(baked_shapes);
}

CommandList Baker::clear() noexcept {
    CommandList ret;
    ret << radiance_.reset();
    ret << final_radiance_.reset();
    ret << batch_mesh_.clear();
    return ret;
}

uint Baker::calculate_buffer_size() noexcept {
    return 2048 * 1024;
}

void Baker::allocate() noexcept {
    uint buffer_size = calculate_buffer_size();
    radiance_ = device().create_buffer<float4>(buffer_size, "bake radiance");
    final_radiance_ = device().create_buffer<float4>(buffer_size, "bake final radiance");
    batch_mesh_.allocate(buffer_size);
}

CommandList Baker::deallocate() noexcept {
    CommandList ret;
    ret << radiance_.reallocate(0)
        << final_radiance_.reallocate(0);
    return ret;
}

}// namespace vision