//
// Created by Zero on 27/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"

namespace vision {

template<EPort p = D, uint N = 4>
[[nodiscard]] oc_uint<p> tea(oc_uint<p> val0, oc_uint<p> val1) {
    oc_uint<p> v0 = val0;
    oc_uint<p> v1 = val1;
    oc_uint<p> s0 = 0;
    for (uint n = 0; n < N; n++) {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}

template<EPort p = D>
[[nodiscard]] oc_float<p> lcg(oc_uint<p> &state) noexcept {
    constexpr auto lcg_a = 1664525u;
    constexpr auto lcg_c = 1013904223u;
    state = lcg_a * state + lcg_c;
    return ((state & 0x00ffffffu) * 1.f) * (1.f / static_cast<float>(0x01000000u));
}

}// namespace vision