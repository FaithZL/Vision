//
// Created by Zero on 14/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "interval.h"
#include "math/base.h"

namespace vision {
using namespace ocarina;

inline namespace math {
template<typename T, size_t N>
struct TBox {
public:
    using scalar_t = T;
    using vector_t = Vector<T, N>;
    vector_t lower, upper;

public:
    TBox() : lower(empty_range_lower<scalar_t>()),
             upper(empty_range_upper<scalar_t>()) {}

    explicit inline TBox(const vector_t &v)
        : lower(v), upper(v) {}

    TBox(const vector_t &lo, const vector_t &hi)
        : lower(lo), upper(hi) {}

    [[nodiscard]] TBox including(const vector_t &other) const {
        return TBox(min(lower, other), max(upper, other));
    }

    [[nodiscard]] TBox including(const TBox &other) const {
        return TBox(min(lower, other.lower), max(upper, other.upper));
    }

    [[nodiscard]] TBox &extend(const vector_t &other) {
        lower = min(lower, other);
        upper = max(upper, other);
        return *this;
    }

    [[nodiscard]] TBox &extend(const TBox &other) {
        lower = min(lower, other.lower);
        upper = max(upper, other.upper);
        return *this;
    }

    [[nodiscard]] interval<scalar_t> get_slab(const uint32_t dim) {
        return interval<scalar_t>(lower[dim], upper[dim]);
    }

    [[nodiscard]] vector_t offset(vector_t p) const {
        return (p - lower) / span();
    }

    [[nodiscard]] bool contains(const vector_t &point) const {
        return all(point >= lower) && all(upper >= point);
    }

    [[nodiscard]] bool contains(const TBox &other) const {
        return all(other.lower >= lower) && all(upper >= other.upper);
    }

    [[nodiscard]] bool overlap(const TBox &other) const {
        return contains(other.lower) || contains(other.upper);
    }

    [[nodiscard]] scalar_t radius() const {
        return length(upper - lower) * 0.5f;
    }

    [[nodiscard]] vector_t lerp(vector_t t) const {
        return ocarina::lerp(t, lower, upper);
    }

    [[nodiscard]] vector_t center() const {
        return (lower + upper) * 0.5f;
    }

    [[nodiscard]] vector_t span() const {
        return upper - lower;
    }

    [[nodiscard]] vector_t size() const {
        return upper - lower;
    }

    [[nodiscard]] scalar_t volume() const {
        return ocarina::volume(upper - lower);
    }

    [[nodiscard]] scalar_t area() const {
        static_assert(N == 2 || N == 3);
        vector_t diag = upper - lower;
        if constexpr (N == 2) {
            return diag.x * diag.y;
        } else if constexpr (N == 3) {
            return 2 * (diag.x * diag.y + diag.x * diag.z + diag.y * diag.z);
        }
    }

    [[nodiscard]] bool empty() const {
        return any(upper < lower);
    }

    [[nodiscard]] auto advance(Vector<scalar_t, 2> p) const {
        ++p.x;
        if (p.x == upper.x) {
            p.x = lower.x;
            ++p.y;
        }
        return p;
    }

    template<typename Func>
    void for_each(Func func) const {
        static_assert(std::is_integral_v<scalar_t> || std::is_unsigned_v<scalar_t>,
                      "scalar_t must be unsigned or integral!");
        auto p = lower;
        do {
            func(p);
            p = advance(p);
        } while (all(p < upper));
    }
};

#define VISION_DEFINE_BOX(scalar_t, suffix)   \
    using Box##2##suffix = TBox<scalar_t, 2>; \
    using Box##3##suffix = TBox<scalar_t, 3>; \
    using Box##4##suffix = TBox<scalar_t, 4>;

VISION_DEFINE_BOX(int32_t, i);
VISION_DEFINE_BOX(uint32_t, u);
VISION_DEFINE_BOX(float, f);
VISION_DEFINE_BOX(double, d);
VISION_DEFINE_BOX(int64_t, l);

#undef VISION_DEFINE_BOX
}// namespace math

}// namespace vision