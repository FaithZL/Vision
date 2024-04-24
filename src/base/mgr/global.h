//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "util/file_manager.h"
#include "image_pool.h"
#include "rhi/common.h"

namespace vision {
class Pipeline;
class SpectrumImpl;
class Global {
private:
    Global() = default;
    Global(const Global &) = delete;
    Global(Global &&) = delete;
    Global operator=(const Global &) = delete;
    Global operator=(Global &&) = delete;
    static Global *s_global;
    ~Global();

private:
    Pipeline *pipeline_{nullptr};
    Device *device_{nullptr};
    fs::path scene_path_;

public:
    [[nodiscard]] static Global &instance();
    static void destroy_instance();
    void set_pipeline(Pipeline *pipeline);
    [[nodiscard]] Pipeline *pipeline();
    [[nodiscard]] ImagePool &image_pool() {
        return ImagePool::instance();
    }
    [[nodiscard]] Device &device() noexcept { return *device_; }
    void set_device(Device *val) noexcept { device_ = val; }
    [[nodiscard]] BindlessArray &bindless_array();
    void set_scene_path(const fs::path &sp) noexcept;
    [[nodiscard]] fs::path scene_path() const noexcept;
    [[nodiscard]] fs::path scene_cache_path() const noexcept;
    [[nodiscard]] static decltype(auto) file_manager() {
        return FileManager::instance();
    }
};

class FrameBuffer;

class Context {
protected:
    Context() = default;

public:
    [[nodiscard]] static Device &device() noexcept;
    [[nodiscard]] static Stream &stream() noexcept;
    [[nodiscard]] static Pipeline *pipeline() noexcept;
    [[nodiscard]] static Scene &scene() noexcept;
    [[nodiscard]] static FrameBuffer &frame_buffer() noexcept;
    [[nodiscard]] static SpectrumImpl &spectrum() noexcept;
};

}// namespace vision