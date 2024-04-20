//
// Created by Zero on 2023/6/25.
//

#pragma once

#include <utility>
#include "rhi/common.h"
#include "core/stl.h"

namespace vision {
using namespace ocarina;

enum ResourceFormat {
    BYTE1,
    BYTE2,
    BYTE4,

    UINT1,
    UINT2,
    UINT4,

    FLOAT1,
    FLOAT2,
    FLOAT4,

    UNKNOWN

};

class RenderResource {
public:
    using Tag = ocarina::RHIResource::Tag;

protected:
    uint width_{};
    uint height_{};
    uint depth_{};
    Tag tag_{};

public:
    RenderResource() = default;
    OC_MAKE_MEMBER_GETTER(width, )
    OC_MAKE_MEMBER_GETTER(height, )
    OC_MAKE_MEMBER_GETTER(depth, )
    [[nodiscard]] virtual const RHIResource *rhi_resource() const noexcept = 0;
    [[nodiscard]] virtual RHIResource *rhi_resource() noexcept = 0;
    virtual ~RenderResource() = default;
};

template<typename T>
class TResource final : public RenderResource {
private:
    T rhi_resource_{};

public:
    explicit TResource(T &&res)
        : rhi_resource_(OC_FORWARD(res)) {}
    [[nodiscard]] const Type *type() const noexcept { return Type::of<T>(); }
    [[nodiscard]] const RHIResource *rhi_resource() const noexcept override { return &rhi_resource_; }
    [[nodiscard]] RHIResource *rhi_resource() noexcept override { return &rhi_resource_; }
};

}// namespace vision