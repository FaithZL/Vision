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
    virtual void deserialize(void *address) const noexcept = 0;
    virtual void serialize(void *address) noexcept = 0;
};

template<typename T>
requires std::derived_from<T, RuntimeObject> || std::is_trivially_copyable_v<T>
class SerializedData : public Serializable {
public:
    using attr_map_t = std::map<ocarina::string_view, UP<Serializable>>;
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

    void serialize(void *address) noexcept override {
        serialize_impl(*reinterpret_cast<data_type *>(address));
    }

    void deserialize(void *address) const noexcept override {
        oc_memcpy(address, addressof(data_), sizeof(data_));
    }
};

class RuntimeObject;

class Serializer {
public:
    using handle_t = const RuntimeObject *;
    using object_map_t = std::map<handle_t, UP<Serializable>>;

private:
    object_map_t object_map_;

public:
    Serializer() = default;

    void erase_old_object(handle_t object) {
        object_map_.erase(object);
    }

    template<typename T>
    [[nodiscard]] Serializable *get_serialized_data(handle_t old_obj) noexcept {
        if (!object_map_.contains(old_obj)) {
            object_map_.insert(make_pair(old_obj, make_unique<SerializedData<T>>()));
        }
        return object_map_.at(old_obj).get();
    }

    template<typename TObject, typename TMember>
    void serialize(const TObject *old_obj, string_view name, const TMember &value) {
        Serializable *serialized_data = get_serialized_data<TObject>(old_obj);
        int i = 0;
    }

    template<typename T>
    void deserialize(handle_t old_obj, ocarina::string_view name, T &value) {
    }
};

}// namespace vision::inline hotfix