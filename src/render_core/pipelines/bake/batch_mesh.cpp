//
// Created by Zero on 2023/7/6.
//

#include "batch_mesh.h"
#include "baker.h"

namespace vision {

CommandList BatchMesh::clear() noexcept {
    CommandList ret;
    ret << _triangles.device_buffer().clear();
    ret << _vertices.device_buffer().clear();
    ret << _pixels.clear();
    ret << [&] {
        _triangles.host_buffer().clear();
        _vertices.host_buffer().clear();
        _triangle_nums.clear();
        _resolutions.clear();
    };
    return ret;
}

void BatchMesh::setup(ocarina::span<BakedShape> baked_shapes, uint buffer_size) noexcept {
    uint vert_offset = 0;
    vector<std::pair<uint2, uint>> res_offset;
    uint pixel_offset = 0u;
    for (const BakedShape &bs : baked_shapes) {
        uint tri_num = 0;
        bs.shape()->for_each_mesh([&](const vision::Mesh &mesh, int index) {
            float4x4 o2w = bs.shape()->o2w();
            for (const Vertex &vertex : mesh.vertices) {
                float3 world_pos = transform_point<H>(o2w, vertex.position());
                float3 world_norm = transform_normal<H>(o2w, vertex.normal());
                world_norm = select(nonzero(world_norm), normalize(world_norm), world_norm);
                _vertices.emplace_back(world_pos, world_norm, vertex.tex_coord(), vertex.lightmap_uv());
            }
            for (const Triangle &tri : mesh.triangles) {
                _triangles.emplace_back(tri.i + vert_offset, tri.j + vert_offset, tri.k + vert_offset);
                res_offset.emplace_back(bs.resolution(), pixel_offset);
            }
            tri_num += mesh.triangles.size();
            vert_offset += mesh.vertices.size();
        });
        _triangle_nums.push_back(tri_num);
        _resolutions.push_back(bs.resolution());
        pixel_offset += bs.pixel_num();
    }

    _vertices.reset_device_buffer_immediately(device());
    _triangles.reset_device_buffer_immediately(device());
    _pixels = device().create_buffer<uint4>(buffer_size);

    CommandList cmd_lst;
    cmd_lst << _vertices.upload()
            << _triangles.upload();
    for (uint i = 0; i < _triangles.host_buffer().size(); ++i) {
        auto [res, offset] = res_offset[i];
        cmd_lst << _rasterizer(_triangles, _vertices,
                               _pixels, offset, i, res)
                       .dispatch(buffer_size);
        cmd_lst << Printer::instance().retrieve();
        cmd_lst << synchronize();
        break;
    }
    stream() << cmd_lst << commit();
    vector<uint4> pd;
    pd.resize(buffer_size);
    _pixels.download_immediately(pd.data());
    int i = 0;
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
        Uint4 pixel{};
        $if(in_triangle<D>(coord, p0, p1, p2)) {
            pixel.x = triangle_index;
        }
        $else {
            pixel.x = InvalidUI32;
        };
        pixel.y = pixel_offset;
        pixel.z = res.x;
        pixel.w = res.y;
        pixels.write(dispatch_id(), pixel);
    };
    _rasterizer = device().compile(kernel, "rasterize");
}

CommandList BatchMesh::rasterize() const noexcept {
    CommandList ret;
    uint pixel_offset = 0;
    uint tri_offset = 0;

    return ret;
}

void BatchMesh::append(const BakedShape &bs) noexcept {
}

}// namespace vision