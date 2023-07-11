//
// Created by Zero on 2023/7/6.
//

#include "batch_mesh.h"
#include "baker.h"
#include "util.h"

namespace vision {

CommandList BatchMesh::clear() noexcept {
    CommandList ret;
    ret << _triangles_old.device_buffer().clear();
    ret << _vertices_old.device_buffer().clear();
    ret << _pixels.device_buffer().clear();
    ret << [&] {
        _triangles_old.host_buffer().clear();
        _vertices_old.host_buffer().clear();
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
    for (BakedShape &bs : baked_shapes) {
        MergedMesh &mesh = bs.merged_mesh();

    }
}

void BatchMesh::setup(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint vert_offset = 0;
    vector<std::pair<uint2, uint>> res_offset;
    for (BakedShape &bs : baked_shapes) {
        bs.shape()->for_each_mesh([&](const vision::Mesh &mesh, int index) {
            float4x4 o2w = bs.shape()->o2w();
            for (const Vertex &vertex : mesh.vertices) {
                float3 world_pos = transform_point<H>(o2w, vertex.position());
                float3 world_norm = transform_normal<H>(o2w, vertex.normal());
                world_norm = select(nonzero(world_norm), normalize(world_norm), world_norm);
                _vertices_old.emplace_back(world_pos, world_norm, vertex.tex_coord(), vertex.lightmap_uv());
            }
            for (const Triangle &tri : mesh.triangles) {
                _triangles_old.emplace_back(tri.i + vert_offset, tri.j + vert_offset, tri.k + vert_offset);
                res_offset.emplace_back(bs.resolution(), _pixel_num);
            }
            vert_offset += mesh.vertices.size();
        });
        bs.normalize_lightmap_uv();
        _pixel_num += bs.pixel_num();
    }

    _vertices_old.reset_device_buffer_immediately(device());
    _triangles_old.reset_device_buffer_immediately(device());

    auto rasterize = [&](){
        stream() << _vertices_old.upload()
                << reset_pixels()
                << _triangles_old.upload();

        for (uint i = 0; i < _triangles_old.host_buffer().size(); ++i) {
            auto [res, offset] = res_offset[i];
            stream() << shader(_triangles_old, _vertices_old,
                                   _pixels, offset, i, res)
                           .dispatch(pixel_num());
            stream() << synchronize() << commit();
        }
    };

    rasterize();

}

void BatchMesh::compile() noexcept {
    Kernel kernel = [&](BufferVar<Triangle> triangles, BufferVar<Vertex> vertices,
                        BufferVar<uint4> pixels, Uint pixel_offset, Uint triangle_index, Uint2 res) {
        Uint pixel_num = res.x * res.y;
        $if(dispatch_id() < pixel_offset || dispatch_id() >= pixel_offset + pixel_num) {
            $return();
        };

        Uint pixel_index = dispatch_id() - pixel_offset;

        Uint x = pixel_index % res.x;
        Uint y = pixel_index / res.x;

        Float2 coord = make_float2(x + 0.5f, y + 0.5f);
        Var tri = triangles.read(triangle_index);
        Var v0 = vertices.read(tri.i);
        Var v1 = vertices.read(tri.j);
        Var v2 = vertices.read(tri.k);

        Float2 p0 = v0->lightmap_uv();
        Float2 p1 = v1->lightmap_uv();
        Float2 p2 = v2->lightmap_uv();
        Uint4 pixel = pixels.read(dispatch_id());
        $if(in_triangle<D>(coord, p0, p1, p2)) {
            pixel.x = triangle_index;
        };
        pixel.y = pixel_offset;
        pixel.z = detail::uint2_to_uint(res);
        pixels.write(dispatch_id(), pixel);
    };
    shader = device().compile(kernel, "rasterize");
}

}// namespace vision