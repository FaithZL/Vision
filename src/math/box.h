//
// Created by Zero on 14/09/2022.
//

#pragma once

#include "core/basic_types.h"

namespace vision {
using namespace ocarina;

template<typename T, size_t N>
struct TBox {
    using scalar_t = T;
    using vector_t = Vector<T, N>;

    vector_t lower, upper;
};

#define _define_box(scalar_t, suffix)         \
    using Box##2##suffix = TBox<scalar_t, 2>; \
    using Box##3##suffix = TBox<scalar_t, 3>; \
    using Box##4##suffix = TBox<scalar_t, 4>;

_define_box(int32_t, i);
_define_box(uint32_t, u);
_define_box(float, f);
_define_box(double, d);
_define_box(int64_t, l);

#undef _define_box

}// namespace vision