//
// Created by Zero on 2023/7/6.
//

#include "batch_mesh.h"
#include "baker.h"
#include "util.h"

namespace vision {

CommandList BatchMesh::clear() noexcept {
    CommandList ret;
    ret << _triangles.device_buffer().clear();
    ret << _vertices.device_buffer().clear();
    ret << _pixels.device_buffer().clear();
    ret << [&] {
        _triangles.host_buffer().clear();
        _vertices.host_buffer().clear();
        _pixel_num = 0;
    };
    return ret;
}

void BatchMesh::allocate(ocarina::uint buffer_size) {
    _pixels.resize(buffer_size);
    for (int i = 0; i < buffer_size; ++i) {
        _pixels.at(i) = make_uint4(InvalidUI32);
    }
    _pixels.device_buffer() = device().create_buffer<uint4>(buffer_size);
}

Command *BatchMesh::reset_pixels() noexcept {
    return _pixels.upload();
}

void BatchMesh::batch(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint triangle_offset = 0;
    uint vert_offset = 0;
    uint pixel_offset = 0;
    CommandList cmd_lst;
    for (BakedShape &bs : baked_shapes) {
        const MergedMesh &mesh = bs.merged_mesh();

        cmd_lst << _shader(bs.pixels(), triangle_offset,
                           pixel_offset, _pixels)
                       .dispatch(bs.resolution());
        for (Triangle tri : mesh.triangles) {
            _triangles.emplace_back(tri.i + vert_offset,
                                    tri.j + vert_offset,
                                    tri.k + vert_offset);
        }
        triangle_offset += mesh.triangles.size();
        vert_offset += mesh.vertices.size();
        pixel_offset += bs.pixel_num();
        append(_vertices, mesh.vertices);
        bs.normalize_lightmap_uv();
        _pixel_num += bs.pixel_num();
    }
    stream() << cmd_lst << synchronize() << commit();
    _pixels.download_immediately();
    _vertices.reset_device_buffer_immediately(device());
    _triangles.reset_device_buffer_immediately(device());
    stream() << _vertices.upload() << _triangles.upload() <<synchronize() << commit();
}

void BatchMesh::compile() noexcept {
    Kernel kernel = [&](BufferVar<uint4> src_pixels, Uint triangle_offset,
                        Uint pixel_offset, BufferVar<uint4> dst_pixels) {
        Uint2 res = dispatch_dim().xy();
        Uint4 pixel = src_pixels.read(dispatch_id());
        Bool valid = bit_cast<uint>(1.f) == pixel.w;
        $if(valid) {
            pixel.x += triangle_offset;
        };
        pixel.y = pixel_offset;
        pixel.z = detail::uint2_to_uint(res);
        dst_pixels.write(dispatch_id() + pixel_offset, pixel);
    };
    _shader = device().compile(kernel, "merge rasterize pixel");
}

}// namespace vision