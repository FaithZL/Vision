//
// Created by Zero on 2024/7/22.
//

#pragma once

#include "math/basic_types.h"
#include "core/stl.h"

namespace vision {

class Serializable {
public:
    virtual ~Serializable() = default;
};

template<typename T>
class SerializedData : public Serializable {
public:
    using data_type = T;

private:
    data_type data_{};

public:
    SerializedData() = default;
    explicit SerializedData(const data_type &v) : data_(v) {}
};

class Serializer {
public:
    using group_type = std::map<ocarina::string_view, Serializable *>;
    using map_type = std::map<ocarina::handle_ty, group_type>;

private:
    group_type group_;
    map_type map_;

public:
    Serializer() = default;

    template<typename T>
    void serialize(ocarina::string_view name, const T &value) {

    }
};

}// namespace vision