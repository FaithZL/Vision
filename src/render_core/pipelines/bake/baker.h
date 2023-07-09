//
// Created by Zero on 2023/7/3.
//

#pragma once

#include "base/bake_utlis.h"
#include "base/mgr/global.h"
#include "dilate_filter.h"
#include "batch_mesh.h"

namespace vision {
#define MAKE_ADD_TIME_FUNC(task) \
    void add_##task##_time(double t) noexcept { task##_time += t; }
#define VS_PLUS_TASK_TIME(task) +task##_time
#define VS_CLEAR_TIME(task) task##_time = 0;
#define VS_MAKE_GET_TIME_STR(task)                                           \
    [[nodiscard]] string get_##task##_stats_str() const noexcept {           \
        return ocarina::format("{} {}", task##_time, task##_time_percent()); \
    }

#define VS_TIME_PERCENT(task) \
    [[nodiscard]] double task##_time_percent() const noexcept { return task##_time / total_time(); }

#define MAKE_TIME_FUNC(...)                            \
    MAP(MAKE_ADD_TIME_FUNC, __VA_ARGS__)               \
    MAP(VS_TIME_PERCENT, __VA_ARGS__)                  \
    MAP(VS_MAKE_GET_TIME_STR, __VA_ARGS__)             \
    [[nodiscard]] double total_time() const noexcept { \
        return 0 MAP(VS_PLUS_TASK_TIME, __VA_ARGS__);  \
    }                                                  \
    void clear_time() noexcept {                       \
        MAP(VS_CLEAR_TIME, __VA_ARGS__)                \
    }
struct BakerStats {
public:
    uint model_num{};
    uint64_t pixel_num{};
    uint batch_num{};
    uint spp{};
    double bake_time{};
    double raster_time{};
    double filter_time{};
    double package_time{};
    double denoise_time{};
    double uv_unwrap_time{};

public:
    MAKE_TIME_FUNC(raster, bake, filter,
                   package, denoise,
                   uv_unwrap)

    [[nodiscard]] string get_scene_stats() const noexcept {
        return ocarina::format("model num is {}, pixel num is {}, batch num is {}, "
                               "{} sample per pixel, total sample num is {}",
                               model_num, pixel_num, batch_num, spp, spp * pixel_num);
    }
};
#undef MAKE_TIME_FUNC
#undef VS_CLEAR_TIME
#undef VS_PLUS_TASK_TIME
#undef MAKE_ADD_TIME_FUNC

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