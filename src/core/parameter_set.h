//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "math/basic_types.h"
#include "core/logging.h"
#include "vs_header.h"

using namespace ocarina;

namespace vision {

using DataWrap = nlohmann::json;

template<typename T, size_t N>
DataWrap to_json(Vector<T, N> vec) {
    DataWrap ret = DataWrap ::array({});
    for (int i = 0; i < N; ++i) {
        ret.push_back(vec[i]);
    }
    return ret;
}

class ParameterSet {
private:
    std::string key_;
    mutable DataWrap data_;

private:
#define VISION_MAKE_AS_TYPE_FUNC(type)     \
    OC_NODISCARD type _as_##type() const { \
        return static_cast<type>(data_);   \
    }

#define VISION_MAKE_AS_TYPE_VEC2(type)                  \
    OC_NODISCARD type##2 _as_##type##2() const {        \
        return make_##type##2(this->at(0).as_##type(),  \
                              this->at(1).as_##type()); \
    }

#define VISION_MAKE_AS_TYPE_VEC3(type)                  \
    OC_NODISCARD type##3 _as_##type##3() const {        \
        return make_##type##3(this->at(0).as_##type(),  \
                              this->at(1).as_##type(),  \
                              this->at(2).as_##type()); \
    }
#define VISION_MAKE_AS_TYPE_VEC4(type)                  \
    OC_NODISCARD type##4 _as_##type##4() const {        \
        return make_##type##4(this->at(0).as_##type(),  \
                              this->at(1).as_##type(),  \
                              this->at(2).as_##type(),  \
                              this->at(3).as_##type()); \
    }

#define VISION_MAKE_AS_TYPE_VEC(type) \
    VISION_MAKE_AS_TYPE_VEC2(type)    \
    VISION_MAKE_AS_TYPE_VEC3(type)    \
    VISION_MAKE_AS_TYPE_VEC4(type)

#define VISION_MAKE_AS_TYPE_MAT(type) \
    VISION_MAKE_AS_TYPE_MAT2X2(type)  \
    VISION_MAKE_AS_TYPE_MAT3X3(type)  \
    VISION_MAKE_AS_TYPE_MAT4X4(type)

#define VISION_MAKE_AS_TYPE_MAT2X2(type)                       \
    OC_NODISCARD float2x2 _as_##type##2x2() const {            \
        if (data_.size() == 2) {                               \
            return make_##type##2x2(                           \
                this->at(0)._as_##type##2(),                   \
                this->at(1)._as_##type##2());                  \
        } else {                                               \
            return make_##type##2x2(this->at(0)._as_##type(),  \
                                    this->at(1)._as_##type(),  \
                                    this->at(2)._as_##type(),  \
                                    this->at(3)._as_##type()); \
        }                                                      \
    }

#define VISION_MAKE_AS_TYPE_MAT3X3(type)                       \
    OC_NODISCARD float3x3 _as_##type##3x3() const {            \
        if (data_.size() == 3) {                               \
            return make_##type##3x3(                           \
                this->at(0)._as_##type##3(),                   \
                this->at(1)._as_##type##3(),                   \
                this->at(2)._as_##type##3());                  \
        } else {                                               \
            return make_##type##3x3(this->at(0)._as_##type(),  \
                                    this->at(1)._as_##type(),  \
                                    this->at(2)._as_##type(),  \
                                    this->at(3)._as_##type(),  \
                                    this->at(4)._as_##type(),  \
                                    this->at(5)._as_##type(),  \
                                    this->at(6)._as_##type(),  \
                                    this->at(7)._as_##type(),  \
                                    this->at(8)._as_##type()); \
        }                                                      \
    }
#define VISION_MAKE_AS_TYPE_MAT4X4(type)                        \
    OC_NODISCARD float4x4 _as_##type##4x4() const {             \
        if (data_.size() == 4) {                                \
            return make_##type##4x4(                            \
                this->at(0)._as_##type##4(),                    \
                this->at(1)._as_##type##4(),                    \
                this->at(2)._as_##type##4(),                    \
                this->at(3)._as_##type##4());                   \
        } else {                                                \
            return make_##type##4x4(this->at(0)._as_##type(),   \
                                    this->at(1)._as_##type(),   \
                                    this->at(2)._as_##type(),   \
                                    this->at(3)._as_##type(),   \
                                    this->at(4)._as_##type(),   \
                                    this->at(5)._as_##type(),   \
                                    this->at(6)._as_##type(),   \
                                    this->at(7)._as_##type(),   \
                                    this->at(8)._as_##type(),   \
                                    this->at(9)._as_##type(),   \
                                    this->at(10)._as_##type(),  \
                                    this->at(11)._as_##type(),  \
                                    this->at(12)._as_##type(),  \
                                    this->at(13)._as_##type(),  \
                                    this->at(14)._as_##type(),  \
                                    this->at(15)._as_##type()); \
        }                                                       \
    }

    VISION_MAKE_AS_TYPE_FUNC(int)
    VISION_MAKE_AS_TYPE_FUNC(uint)
    VISION_MAKE_AS_TYPE_FUNC(bool)
    VISION_MAKE_AS_TYPE_FUNC(float)
    VISION_MAKE_AS_TYPE_FUNC(string)
    VISION_MAKE_AS_TYPE_VEC(uint)
    VISION_MAKE_AS_TYPE_VEC(int)
    VISION_MAKE_AS_TYPE_VEC(float)
    VISION_MAKE_AS_TYPE_MAT(float)

#undef VISION_MAKE_AS_TYPE_FUNC

#undef VISION_MAKE_AS_TYPE_VEC
#undef VISION_MAKE_AS_TYPE_VEC2
#undef VISION_MAKE_AS_TYPE_VEC3
#undef VISION_MAKE_AS_TYPE_VEC4

#undef VISION_MAKE_AS_TYPE_MAT
#undef VISION_MAKE_AS_TYPE_MAT3X3
#undef VISION_MAKE_AS_TYPE_MAT4X4

public:
    ParameterSet() = default;

    ParameterSet(const DataWrap &json, const string &key = "")
        : data_(json), key_(key) {}

    void set_json(const DataWrap &json) { data_ = json; }

    OC_NODISCARD DataWrap data() const { return data_; }

    OC_NODISCARD ParameterSet get(const std::string &key) const {
        return ParameterSet(data_[key], key);
    }

    OC_NODISCARD ParameterSet at(uint idx) const {
        return ParameterSet(data_.at(idx));
    }

    OC_NODISCARD ParameterSet operator[](const std::string &key) const {
        return ParameterSet(data_.value(key, DataWrap()), key);
    }

    void set_value(const std::string &key, const DataWrap &data) noexcept {
        data_[key] = data;
    }

    void set_value_if_null(const std::string &key, const DataWrap &data) noexcept {
        if (data_.contains(key)) {
            return;
        }
        data_[key] = data;
    }

    OC_NODISCARD ParameterSet value(const string &key,
                                    const DataWrap &data = DataWrap::object()) const {
        return ParameterSet(data_.value(key, data), key);
    }

    OC_NODISCARD ParameterSet operator[](uint i) const {
        return ParameterSet(data_[i]);
    }

    template<typename... Args>
    [[nodiscard]] bool contains(Args &&...args) const noexcept {
        return data_.contains(OC_FORWARD(args)...);
    }

    template<typename T>
    OC_NODISCARD std::vector<T> as_vector() const {
        OC_ERROR_IF(!(data_.is_array() || data_.is_number()), "data is not array!");
        std::vector<T> ret;
        if (data_.is_array()) {
            for (const auto &elm : data_) {
                ParameterSet ps{elm};
                ret.push_back(ps.template as<T>());
            }
        } else if (data_.is_number()) {
            ret.push_back(data_);
        }
        return ret;
    }

#define VISION_MAKE_AS_TYPE_SCALAR(type)                   \
    OC_NODISCARD type as_##type(type val = type()) const { \
        try {                                              \
            if (data_.is_null()) data_ = val;              \
            return _as_##type();                           \
        } catch (const std::exception &e) {                \
            return val;                                    \
        }                                                  \
    }                                                      \
    template<typename T>                                   \
    requires std::is_same_v<T, type>                       \
    [[nodiscard]] T as(T t = T{}) const {                  \
        return as_##type(t);                               \
    }

#define VISION_MAKE_AS_TYPE_VEC_DIM(type, dim)                                                 \
    OC_NODISCARD type##dim as_##type##dim(type##dim val = make_##type##dim()) const noexcept { \
        try {                                                                                  \
            return _as_##type##dim();                                                          \
        } catch (const std::exception &e) {                                                    \
            return val;                                                                        \
        }                                                                                      \
    }                                                                                          \
    template<typename T>                                                                       \
    requires std::is_same_v<T, type##dim>                                                      \
    [[nodiscard]] T as(T t = T{}) const {                                                      \
        return as_##type##dim(t);                                                              \
    }

#define VISION_MAKE_AS_TYPE_MAT_DIM(type, dim)                                                                                 \
    OC_NODISCARD type##dim##x##dim as_##type##dim##x##dim(type##dim##x##dim val = make_##type##dim##x##dim()) const noexcept { \
        try {                                                                                                                  \
            return _as_##type##dim##x##dim();                                                                                  \
        } catch (const std::exception &e) {                                                                                    \
            return val;                                                                                                        \
        }                                                                                                                      \
    }                                                                                                                          \
    template<typename T>                                                                                                       \
    requires std::is_same_v<T, type##dim##x##dim>                                                                              \
    [[nodiscard]] T as(T t = T{}) const {                                                                                      \
        return as_##type##dim##x##dim(t);                                                                                      \
    }

#define VISION_MAKE_AS_TYPE_MAT(type)    \
    VISION_MAKE_AS_TYPE_MAT_DIM(type, 2) \
    VISION_MAKE_AS_TYPE_MAT_DIM(type, 3) \
    VISION_MAKE_AS_TYPE_MAT_DIM(type, 4)

    VISION_MAKE_AS_TYPE_MAT(float)
    VISION_MAKE_AS_TYPE_SCALAR(float)
    VISION_MAKE_AS_TYPE_SCALAR(uint)
    VISION_MAKE_AS_TYPE_SCALAR(bool)
    VISION_MAKE_AS_TYPE_SCALAR(int)
    VISION_MAKE_AS_TYPE_SCALAR(string)

#define VISION_MAKE_AS_TYPE_VEC(type)    \
    VISION_MAKE_AS_TYPE_VEC_DIM(type, 2) \
    VISION_MAKE_AS_TYPE_VEC_DIM(type, 3) \
    VISION_MAKE_AS_TYPE_VEC_DIM(type, 4)
    VISION_MAKE_AS_TYPE_VEC(int)
    VISION_MAKE_AS_TYPE_VEC(uint)
    VISION_MAKE_AS_TYPE_VEC(float)

#undef VISION_MAKE_AS_TYPE_SCALAR
#undef VISION_MAKE_AS_TYPE_VEC
#undef VISION_MAKE_AS_TYPE_VEC_DIM
#undef VISION_MAKE_AS_TYPE_MAT
#undef VISION_MAKE_AS_TYPE_MAT_DIM
};

}// namespace vision