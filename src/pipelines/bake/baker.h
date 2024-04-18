//
// Created by Zero on 2023/7/3.
//

#pragma once

#include "base/bake_utlis.h"
#include "base/mgr/global.h"
#include "dilate_filter.h"
#include "batch_mesh.h"
#include "util.h"

namespace vision {

class Baker : public Context {
private:
    BakerStats &baker_stats_;
    Buffer<float4> radiance_;
    Buffer<float4> final_radiance_;
    BatchMesh batch_mesh_;
    DilateFilter dilate_filter_{};
    SP<Rasterizer> rasterizer_{};
    Shader<void(uint, Buffer<Triangle>, Buffer<Vertex>, Buffer<uint4>, Buffer<float4>)> _baker;

private:
    void _compile_bake() noexcept;
    void _prepare(ocarina::span<BakedShape> baked_shapes) noexcept;
    void _baking() noexcept;
    void _save_result(ocarina::span<BakedShape> baked_shapes) noexcept;

    /// device function
    [[nodiscard]] tuple<Float3, Float3, Bool, Float> fetch_geometry_data(const BufferVar<Triangle> &triangles,
                                                                         const BufferVar<Vertex> &vertices,
                                                                         const BufferVar<uint4> &pixels,
                                                                         Float2 *p_film) noexcept;
    /// device function
    [[nodiscard]] RayState generate_ray(const Float3 &position,
                                        const Float3 &normal, Float *pdf) const noexcept;

public:
    explicit Baker(BakerStats &baker_stats, const SP<Rasterizer> &rasterizer)
        : baker_stats_(baker_stats), rasterizer_(rasterizer) {}
    void compile() noexcept;
    void allocate() noexcept;
    [[nodiscard]] CommandList deallocate() noexcept;
    [[nodiscard]] uint buffer_size() const noexcept { return radiance_.size(); }
    void baking(ocarina::span<BakedShape> baked_shapes) noexcept;
    [[nodiscard]] CommandList clear() noexcept;
    [[nodiscard]] static uint calculate_buffer_size() noexcept;
};

}// namespace vision