//
// Created by Zero on 2023/7/3.
//

#pragma once

#include "base/bake_utlis.h"

namespace vision {

struct Baker {
private:
    Buffer<float4> _positions;
    Buffer<float4> _normals;
    Buffer<float4> _radiance;
    vector<uint> _pixel_num;

public:
    Baker() = default;
    void allocate(uint buffer_size, Device &device) noexcept;
    [[nodiscard]] CommandList append_buffer(const Buffer<float4> &normals,
                                            const Buffer<float4> &positions) noexcept;
    [[nodiscard]] CommandList clear() noexcept;
    [[nodiscard]] uint pixel_num() const noexcept;
    [[nodiscard]] BufferDownloadCommand *download_radiance(void *ptr, uint offset) const noexcept;
};

}// namespace vision