//
// Created by Zero on 2023/7/3.
//

#include "baker.h"
#include "base/mgr/global.h"

namespace vision {

uint Baker::calculate_buffer_size() noexcept {
    return 2048 * 1024;
}

void Baker::allocate() noexcept {
    uint buffer_size = calculate_buffer_size();
    _positions = device().create_buffer<float4>(buffer_size);
    _normals = device().create_buffer<float4>(buffer_size);
    _radiance = device().create_buffer<float4>(buffer_size);
}

Device &Baker::device() noexcept {
    return pipeline()->device();
}

Scene &Baker::scene() noexcept {
    return pipeline()->scene();
}

Pipeline *Baker::pipeline() noexcept {
    return Global::instance().pipeline();
}

Stream &Baker::stream() noexcept {
    return pipeline()->stream();
}

void Baker::_compile_bake() noexcept {
    Kernel kernel = [&](Uint frame_index, BufferVar<float4> positions,
                        BufferVar<float4> normals, BufferVar<float4> radiance) {
        Float4 position = positions.read(dispatch_id());
        Float4 normal = normals.read(dispatch_id());
        Uint offset = as<uint>(position.w);
        Uint res_x = as<uint>(normal.w);
        $if(res_x != 0u) {
            radiance.write(dispatch_id(), make_float4(1,0.7,0,1));
        };
    };
    _bake_shader = device().compile(kernel, "baker");
}

void Baker::_compile_transform() noexcept {
    Kernel kernel = [&](BufferVar<float4> positions,
                        BufferVar<float4> normals, Float4x4 o2w,
                        Uint offset, Uint2 res) {
        Float4 position = positions.read(dispatch_id());
        Float4 normal = normals.read(dispatch_id());
        $if(position.w > 0.f) {
            Float3 world_pos = transform_point(o2w, position.xyz());
            Float3 world_norm = normalize(transform_normal(o2w, normal.xyz()));
            positions.write(dispatch_id(), make_float4(world_pos, as<float>(offset)));
            normals.write(dispatch_id(), make_float4(world_norm, as<float>(res.x)));
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
                                      bs.shape()->o2w(), offset, bs.resolution())
                        .dispatch(bs.resolution()) << Printer::instance().retrieve();
        stream() << append_buffer(bs.normals(), bs.positions());
        stream() << synchronize() << commit();
        offset += bs.pixel_num();
    }
}

void Baker::_baking() noexcept {
    Sampler *sampler = scene().sampler();
    for (uint i = 0; i < sampler->sample_per_pixel(); ++i) {
        stream() << _bake_shader(i, _positions,
                                 _normals, _radiance)
                        .dispatch(2048 * 1024);
    }
    stream() << Printer::instance().retrieve() << synchronize() << commit();
}

void Baker::_save_result(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint offset = 0;
    for (BakedShape &bs : baked_shapes) {
        Context::create_directory_if_necessary(bs.instance_cache_directory());
        bs.reallocate_lightmap();
        stream() << _radiance.copy_to(bs.lightmap().super(), offset);
        stream() << bs.save_lightmap_to_cache();
        stream() << synchronize() << commit();
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

BufferDownloadCommand *Baker::download_radiance(void *ptr, ocarina::uint offset) const noexcept {
    return _radiance.download(ptr, offset);
}

}// namespace vision