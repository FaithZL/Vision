//
// Created by Zero on 2023/7/3.
//

#pragma once

#include "base/bake_utlis.h"
#include "base/mgr/global.h"
#include "dilate_filter.h"
#include "batch_mesh.h"

namespace vision {

namespace detail {
[[nodiscard]] inline Float uint2_to_float(const Uint2 &arg) noexcept {
    Uint a = arg.x;
    Uint b = arg.y;
    a = a << 16;
    return as<float>(a | b);
}

[[nodiscard]] inline Uint2 float_to_uint2(const Float &f) noexcept {
    Uint arg = as<uint>(f);
    Uint a = (0xffff0000 & arg) >> 16;
    Uint b = 0x0000ffff & arg;
    return make_uint2(a, b);
}

[[nodiscard]] inline Bool is_valid(const Float4 &normal) noexcept {
    return nonzero(normal.xyz());
}

}// namespace detail

class Baker : public Ctx {
private:
    Buffer<float4> _radiance;
    Buffer<float4> _final_radiance;
    vector<uint> _pixel_num;
    BatchMesh _batch_mesh{};
    DilateFilter _dilate_filter{};
    Shader<void(uint, Buffer<float4>, Buffer<float4>, Buffer<float4>)> _bake_shader;
    Shader<void(uint, Buffer<Triangle>, Buffer<Vertex>, Buffer<uint4>, Buffer<float4>)> _baker;

private:
    void _compile_bake() noexcept;
    void _prepare(ocarina::span<BakedShape> baked_shapes) noexcept;
    void _baking() noexcept;
    void _save_result(ocarina::span<BakedShape> baked_shapes) noexcept;
    [[nodiscard]] RayState generate_ray(const Float3 &position,
                                        const Float3 &normal, Float *pdf) const noexcept;

public:
    explicit Baker(Rasterizer *rasterizer) {}
    void compile() noexcept;
    void allocate() noexcept;
    [[nodiscard]] CommandList deallocate() noexcept;
    [[nodiscard]] uint buffer_size() const noexcept { return _radiance.size(); }
    void baking(ocarina::span<BakedShape> baked_shapes) noexcept;
    [[nodiscard]] CommandList clear() noexcept;
    [[nodiscard]] static uint calculate_buffer_size() noexcept;
    [[nodiscard]] uint pixel_num() const noexcept;
};

}// namespace vision