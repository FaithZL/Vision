//
// Created by Zero on 2024/7/22.
//

#pragma once

#include "math/basic_types.h"
#include "core/stl.h"
#include "object.h"

namespace vision::inline hotfix {

using namespace ocarina;
class Serializable {
public:
    virtual ~Serializable() = default;
    virtual void serialize(string_view field_name, SP<Serializable> serializable) = 0;
};

template<typename T>
requires std::derived_from<T, RuntimeObject> || std::is_trivially_copyable_v<T>
class SerializedData : public Serializable {
public:
    using attr_map_t = std::map<ocarina::string_view, SP<Serializable>>;
    static constexpr bool is_pod = std::is_trivially_copyable_v<T>;

private:
    template<typename U>
    struct Object {
        static_assert(always_false_v<U>);
    };

    template<typename U>
    requires std::derived_from<U, RuntimeObject>
    struct Object<U> {
        using type = attr_map_t;
    };

    template<typename U>
    requires std::is_trivially_copyable_v<U>
    struct Object<U> {
        using type = U;
    };

private:
    using data_type = Object<T>::type;
    data_type data_{};
    static constexpr auto size = sizeof(data_);

public:
    SerializedData() = default;
    explicit SerializedData(const data_type &v) {
        serialize_impl(v);
    }

    void serialize_impl(const data_type &value) {
        if constexpr (is_pod) {
            oc_memcpy(addressof(data_), addressof(value), sizeof(data_));
        }
    }

    void serialize(std::string_view field_name, SP<Serializable> serializable) override {
        if constexpr (is_pod) {

        } else {
            data_.insert(make_pair(field_name, serializable));
        }
    }
};

class RuntimeObject;

class Serializer {
public:
    using handle_t = const RuntimeObject *;
    using object_map_t = std::map<handle_t, SP<Serializable>>;

private:
    object_map_t object_map_;

public:
    Serializer() = default;

    void erase_old_object(handle_t object) {
        object_map_.erase(object);
    }

    template<typename T>
    [[nodiscard]] SP<Serializable> get_serialized_data(const T *old_obj) noexcept {
        if (!object_map_.contains(old_obj)) {
            object_map_.insert(make_pair(old_obj, make_shared<SerializedData<T>>()));
        }
        return object_map_.at(old_obj);
    }

    template<typename T>
    void serialize(T &&object) noexcept {
        auto ptr = raw_ptr(OC_FORWARD(object));
        SP<Serializable> data = get_serialized_data(ptr);
        ptr->serialize(data);
    }
};

}// namespace vision::inline hotfix