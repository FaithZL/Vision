//
// Created by Zero on 2023/7/3.
//

#pragma once

#include "base/bake_utlis.h"
#include "base/mgr/global.h"
#include "dilate_filter.h"
#include "batch_mesh.h"

namespace vision {

class Baker : public Ctx {
private:
    Buffer<float4> _radiance;
    Buffer<float4> _final_radiance;
    BatchMesh _batch_mesh{};
    DilateFilter _dilate_filter{};
    Shader<void(uint, Buffer<Triangle>, Buffer<Vertex>, Buffer<uint4>, Buffer<float4>)> _baker;

private:
    void _compile_bake() noexcept;
    void _prepare(ocarina::span<BakedShape> baked_shapes) noexcept;
    void _baking() noexcept;
    void _save_result(ocarina::span<BakedShape> baked_shapes) noexcept;

    /// device function
    [[nodiscard]] tuple<Float3, Float3, Bool> fetch_geometry_data(const BufferVar<Triangle> &triangles,
                                                           const BufferVar<Vertex> &vertices,
                                                           const BufferVar<uint4> &pixels) noexcept;
    /// device function
    [[nodiscard]] RayState generate_ray(const Float3 &position,
                                        const Float3 &normal, Float *pdf) const noexcept;

public:
    Baker() = default;
    void compile() noexcept;
    void allocate() noexcept;
    [[nodiscard]] CommandList deallocate() noexcept;
    [[nodiscard]] uint buffer_size() const noexcept { return _radiance.size(); }
    void baking(ocarina::span<BakedShape> baked_shapes) noexcept;
    [[nodiscard]] CommandList clear() noexcept;
    [[nodiscard]] static uint calculate_buffer_size() noexcept;
};

}// namespace vision