//
// Created by Zero on 2023/7/3.
//

#pragma once

#include "base/bake_utlis.h"
#include "base/mgr/global.h"

namespace vision {

class Baker : public Ctx {
private:
    Buffer<float4> _positions;
    Buffer<float4> _normals;
    Buffer<float4> _radiance;
    vector<uint> _pixel_num;
    Rasterizer *_rasterizer{};
    Shader<void(uint, Buffer<float4>, Buffer<float4>, Buffer<float4>)> _bake_shader;
    Shader<void(Buffer<float4>, Buffer<float4>, float4x4, uint, uint2)> _transform_shader;

private:
    void _compile_transform() noexcept;
    void _compile_bake() noexcept;
    void _prepare(ocarina::span<BakedShape> baked_shapes) noexcept;
    void _baking() noexcept;
    void _save_result(ocarina::span<BakedShape> baked_shapes) noexcept;
    [[nodiscard]] RayState generate_ray(const Float4 &position,
                                        const Float4 &normal, Float *pdf) const noexcept;

public:
    explicit Baker(Rasterizer *rasterizer)
        : _rasterizer(rasterizer) {}
    void compile() noexcept;
    void allocate() noexcept;
    [[nodiscard]] CommandList deallocate() noexcept;
    [[nodiscard]] uint buffer_size() const noexcept { return _normals.size(); }
    void baking(ocarina::span<BakedShape> baked_shapes) noexcept;
    [[nodiscard]] CommandList append_buffer(const Buffer<float4> &normals,
                                            const Buffer<float4> &positions) noexcept;
    [[nodiscard]] CommandList clear() noexcept;
    [[nodiscard]] static uint calculate_buffer_size() noexcept;
    [[nodiscard]] uint pixel_num() const noexcept;
};

}// namespace vision