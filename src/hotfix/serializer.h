//
// Created by Zero on 2024/7/22.
//

#pragma once

#include "math/basic_types.h"
#include "core/stl.h"

namespace vision::inline hotfix {

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

class RuntimeObject;

using namespace ocarina;

class Serializer {
public:
    using attr_map_t = std::map<ocarina::string_view, UP<Serializable>>;
    using object_map_t = std::map<RuntimeObject *, attr_map_t>;

private:
    attr_map_t attr_map_;
    object_map_t object_map_;

private:
    [[nodiscard]] attr_map_t &get_attr_map(RuntimeObject *object) noexcept {
        if (!object_map_.contains(object)) {
            object_map_.insert(make_pair(object, attr_map_t{}));
        }
        return object_map_.at(object);
    }

public:
    Serializer() = default;

    void erase_old_object(RuntimeObject *object) {
        object_map_.erase(object);
    }

    template<typename T>
    void serialize(RuntimeObject *old_obj, ocarina::string_view name, const T &value) {
        attr_map_t &attr_map = get_attr_map(old_obj);
        attr_map.insert(make_pair(name, make_unique<SerializedData<T>>(value)));
    }

    template<typename T>
    [[nodiscard]] T deserialize(RuntimeObject *old_obj, ocarina::string_view name) {
        attr_map_t &attr_map = get_attr_map(old_obj);

    }
};

}// namespace vision