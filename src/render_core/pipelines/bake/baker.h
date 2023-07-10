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
    void add_##task##_time(double t) noexcept { _##task##_time += t; }
#define VS_PLUS_TASK_TIME(task) +_##task##_time
#define VS_PLUS_TASK_TIME_STR(task) +get_##task##_stats_str()
#define VS_CLEAR_TIME(task) _##task##_time = 0;
#define VS_MAKE_GET_TIME_STR(task)                                                  \
    [[nodiscard]] string get_##task##_stats_str() const noexcept {                  \
        return ocarina::format(#task " time is {:.4f}s, proportion is {:2.2f}% \n", \
                               _##task##_time, task##_time_percent() * 100);        \
    }
#define VS_MAKE_GET_TIME_TOTAL_STR(...)                                      \
    [[nodiscard]] string get_total_time_stats() const noexcept {             \
        return get_total_time_str() MAP(VS_PLUS_TASK_TIME_STR, __VA_ARGS__); \
    }

#define VS_TIME_PERCENT(task) \
    [[nodiscard]] double task##_time_percent() const noexcept { return _##task##_time / total_time(); }

#define MAKE_TIME_FUNC(...)                            \
    MAP(MAKE_ADD_TIME_FUNC, __VA_ARGS__)               \
    MAP(VS_TIME_PERCENT, __VA_ARGS__)                  \
    MAP(VS_MAKE_GET_TIME_STR, __VA_ARGS__)             \
    VS_MAKE_GET_TIME_TOTAL_STR(__VA_ARGS__)            \
    [[nodiscard]] double total_time() const noexcept { \
        return 0 MAP(VS_PLUS_TASK_TIME, __VA_ARGS__);  \
    }                                                  \
    void clear_time() noexcept {                       \
        MAP(VS_CLEAR_TIME, __VA_ARGS__)                \
    }

struct BakerStats {
private:
    uint _model_num{};
    size_t _pixel_num{};
    uint _batch_index{};
    uint _spp{};
    double _bake_time{};
    double _raster_time{};
    double _filter_time{};
    double _package_time{};
    double _denoise_time{};
    double _uv_unwrap_time{};
    double _save_time{};

public:
    MAKE_TIME_FUNC(uv_unwrap, raster,
                   bake, filter,
                   package, denoise,
                   save)

    [[nodiscard]] string get_scene_stats() const noexcept {
        return ocarina::format("\nmodel num is {}, \npixel num is {},\nbatch num is {},\n"
                               "{} sample per pixel, \ntotal sample num is {} \n \n",
                               _model_num, _pixel_num, _batch_index, _spp, _spp * _pixel_num);
    }
    void on_batch_start() noexcept {
        _batch_index += 1;
    }
    OC_MAKE_MEMBER_SETTER(spp)
    OC_MAKE_MEMBER_SETTER(model_num)
    OC_MAKE_MEMBER_SETTER(pixel_num)
    void report_progress() const noexcept;
    [[nodiscard]] string get_total_time_str() const noexcept {
        return ocarina::format("total time is {:.4f}s, \n", total_time());
    }
    [[nodiscard]] string get_all_stats() const noexcept {
        return get_scene_stats() + get_total_time_stats();
    }
    void clear() noexcept { *this = BakerStats(); }
    [[nodiscard]] bool is_valid() const noexcept { return _model_num > 0; }
};

struct BakerGuard {
    Clock clock;
    std::function<void(double)> func;
    explicit BakerGuard(const std::function<void(double)> &f)
        : func(f) {}
    ~BakerGuard() {
        func(clock.elapse_s());
    }
};

#define VS_BAKER_STATS(stats, task) \
    BakerGuard __bg([&](double t) { \
        stats.add_##task##_time(t); \
    });

#undef MAKE_TIME_FUNC
#undef VS_CLEAR_TIME
#undef VS_PLUS_TASK_TIME_STR
#undef VS_TIME_PERCENT
#undef VS_MAKE_GET_TIME_TOTAL_STR
#undef VS_MAKE_GET_TIME_STR
#undef VS_PLUS_TASK_TIME
#undef MAKE_ADD_TIME_FUNC

class Baker : public Ctx {
private:
    BakerStats &_baker_stats;
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
    explicit Baker(BakerStats &baker_stats)
        : _baker_stats(baker_stats) {}
    void compile() noexcept;
    void allocate() noexcept;
    [[nodiscard]] CommandList deallocate() noexcept;
    [[nodiscard]] uint buffer_size() const noexcept { return _radiance.size(); }
    void baking(ocarina::span<BakedShape> baked_shapes) noexcept;
    [[nodiscard]] CommandList clear() noexcept;
    [[nodiscard]] static uint calculate_buffer_size() noexcept;
};

}// namespace vision