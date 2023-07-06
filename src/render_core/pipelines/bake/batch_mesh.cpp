//
// Created by Zero on 2023/7/6.
//

#include "batch_mesh.h"
#include "baker.h"

namespace vision {

CommandList BatchMesh::clear() noexcept {
    CommandList ret;
    ret << _triangle.device_buffer().clear();
    ret << _vertices.device_buffer().clear();
    ret << _pixels.clear();
    ret << [&] {
        _triangle.host_buffer().clear();
        _vertices.host_buffer().clear();
        _triangle_nums.clear();
        _pixel_nums.clear();
    };
    return ret;
}

void BatchMesh::compile() noexcept {
    Kernel kernel = [&](BufferVar<Triangle> triangle, BufferVar<Vertex> vertices,
                        BufferVar<Pixel> pixels, Uint pixel_offset, Uint triangle_index, Uint2 res) {

    };
    _rasterizer = device().compile(kernel, "rasterize");
}

void BatchMesh::setup(ocarina::span<BakedShape> baked_shapes) noexcept {
    for (const BakedShape &bs : baked_shapes) {
        append(bs);
    }
}

CommandList BatchMesh::rasterize() const noexcept {
    CommandList ret;
    uint pixel_offset = 0;
    uint tri_offset = 0;
    for (int i = 0; i < _pixel_nums.size(); ++i) {
        uint tri_num = _triangle_nums[i];
        uint pixel_num = _pixel_nums[i];
        for (int j = 0; j < tri_num; ++j) {
//            ret << _rasterizer(_triangle, _vertices, _pixels, pixel_offset, j + tri_offset).dispatch(tri_num);
        }
        tri_offset += tri_num;
        pixel_offset += pixel_num;
    }
    return ret;
}

void BatchMesh::append(const BakedShape &bs) noexcept {
    bs.shape()->for_each_mesh([&](const vision::Mesh &mesh, int index) {
        float4x4 o2w = bs.shape()->o2w();
        for (const Vertex &vertex : mesh.vertices) {
            float3 world_pos = transform_point<H>(o2w, vertex.position());
            float3 world_norm = transform_normal<H>(o2w, vertex.normal());
            world_norm = select(nonzero(world_norm), normalize(world_norm), world_norm);
            _vertices.emplace_back(world_pos, world_norm, vertex.tex_coord(), vertex.lightmap_uv());
        }
    });
}

}// namespace vision