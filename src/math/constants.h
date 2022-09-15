//
// Created by Zero on 15/09/2022.
//

#pragma once

#include "core/stl.h"
#include "core/basic_types.h"

namespace vision {
inline namespace constants {

struct NegInfTy {
    operator double() const { return -std::numeric_limits<double>::infinity(); }
    operator float() const { return -std::numeric_limits<float>::infinity(); }
    operator long long() const { return std::numeric_limits<long long>::min(); }
    operator unsigned long long() const { return std::numeric_limits<unsigned long long>::min(); }
    operator long() const { return std::numeric_limits<long>::min(); }
    operator unsigned long() const { return std::numeric_limits<unsigned long>::min(); }
    operator int() const { return std::numeric_limits<int>::min(); }
    operator unsigned int() const { return std::numeric_limits<unsigned int>::min(); }
    operator short() const { return std::numeric_limits<short>::min(); }
    operator unsigned short() const { return std::numeric_limits<unsigned short>::min(); }
    operator char() const { return std::numeric_limits<char>::min(); }
    operator unsigned char() const { return std::numeric_limits<unsigned char>::min(); }
};

struct PosInfTy {
    operator double() const { return std::numeric_limits<double>::infinity(); }
    operator float() const { return std::numeric_limits<float>::infinity(); }
    operator long long() const { return std::numeric_limits<long long>::max(); }
    operator unsigned long long() const { return std::numeric_limits<unsigned long long>::max(); }
    operator long() const { return std::numeric_limits<long>::max(); }
    operator unsigned long() const { return std::numeric_limits<unsigned long>::max(); }
    operator int() const { return std::numeric_limits<int>::max(); }
    operator unsigned int() const { return std::numeric_limits<unsigned int>::max(); }
    operator short() const { return std::numeric_limits<short>::max(); }
    operator unsigned short() const { return std::numeric_limits<unsigned short>::max(); }
    operator char() const { return std::numeric_limits<char>::max(); }
    operator unsigned char() const { return std::numeric_limits<unsigned char>::max(); }
};

struct NaNTy {
    operator double() const { return std::numeric_limits<double>::quiet_NaN(); }
    operator float() const { return std::numeric_limits<float>::quiet_NaN(); }
};

struct UlpTy {
    operator double() const { return std::numeric_limits<double>::epsilon(); }
    operator float() const { return std::numeric_limits<float>::epsilon(); }
};

template<typename T>
[[nodiscard]] T empty_range_lower() {
    return (T)NegInfTy();
}

template<typename T>
[[nodiscard]] T empty_range_upper() {
    return (T)PosInfTy();
}

}
}// namespace vision::constants