//
// Created by Zero on 2023/6/25.
//

#pragma once

#include <utility>
#include "rhi/common.h"
#include "core/stl.h"

namespace vision {
using namespace ocarina;

class RenderResource {
public:
    using Tag = ocarina::RHIResource::Tag;

protected:
    uint _width{};
    uint _height{};
    uint _depth{};
    Tag _tag{};

public:
    RenderResource() = default;
    OC_MAKE_MEMBER_GETTER(width, )
    OC_MAKE_MEMBER_GETTER(height, )
    OC_MAKE_MEMBER_GETTER(depth, )
};

template<typename T>
class TResource final : public RenderResource {
private:
    T _rhi_resource{};

public:
    explicit TResource(T &&res)
        : _rhi_resource(OC_FORWARD(res)) {}
    OC_MAKE_MEMBER_GETTER(rhi_resource, &)
    [[nodiscard]] const Type *type() const noexcept { return Type::of<T>(); }
};

}// namespace vision