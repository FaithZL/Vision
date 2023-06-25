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
private:
    string _name;
    const Type *_type{};
    RHIResource *_rhi_resource{};

public:
    explicit RenderResource(string name = "")
        : _name(std::move(name)) {}

    template<typename T>
    void emplace(const T &t) noexcept {
        _rhi_resource = &t;
        _type = Type::of<T>();
    }
    OC_MAKE_MEMBER_GETTER(name,)
    OC_MAKE_MEMBER_GETTER(type,)
};

}// namespace vision