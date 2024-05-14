//
// Created by Zero on 2023/7/6.
//

#include "batch_mesh.h"
#include "baker.h"
#include "util.h"

namespace vision {

CommandList BatchMesh::clear() noexcept {
    CommandList ret;
    ret << triangles_.reset();
    ret << vertices_.reset();
    ret << pixels_.reset();
    ret << [&] {
        pixel_num_ = 0;
    };
    return ret;
}

void BatchMesh::allocate(ocarina::uint buffer_size) {
    pixels_ = device().create_buffer<uint4>(buffer_size, "batch mesh pixel");
}

void BatchMesh::batch(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint triangle_offset = 0;
    uint vert_offset = 0;
    uint pixel_offset = 0;

    vector<Triangle> triangles;
    vector<Vertex> vertices;

    CommandList cmd_lst;
    for (BakedShape &bs : baked_shapes) {
        const MergedMesh &mesh = bs.merged_mesh();

        cmd_lst << _shader(bs.pixels(), triangle_offset,
                           pixel_offset, pixels_)
                       .dispatch(bs.resolution());
        for (Triangle tri : mesh.triangles) {
            triangles.emplace_back(tri.i + vert_offset,
                                   tri.j + vert_offset,
                                   tri.k + vert_offset);
        }
        triangle_offset += mesh.triangles.size();
        vert_offset += mesh.vertices.size();
        pixel_offset += bs.pixel_num();
        append(vertices, mesh.vertices);
        pixel_num_ += bs.pixel_num();
        cmd_lst << bs.pixels().reset();
    }
    stream() << cmd_lst << synchronize() << commit();
    vertices_ = device().create_buffer<Vertex>(vertices.size(), "batched mesh vertices");
    triangles_ = device().create_buffer<Triangle>(triangles.size(), "batched mesh triangles");
    stream() << vertices_.upload(vertices.data())
             << triangles_.upload(triangles.data())
             << synchronize() << commit();
}

void BatchMesh::compile() noexcept {
    Kernel kernel = [&](BufferVar<uint4> src_pixels, Uint triangle_offset,
                        Uint pixel_offset, BufferVar<uint4> dst_pixels) {
        Uint2 res = dispatch_dim().xy_();
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