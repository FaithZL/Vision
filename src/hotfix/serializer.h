//
// Created by Zero on 2024/7/22.
//

#pragma once

#include "math/basic_types.h"
#include "core/stl.h"

namespace vision::inline hotfix {

class RuntimeObject;

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
    using data_type = T;
    static constexpr bool is_pod = std::is_trivially_copyable_v<T>;

private:
    data_type data_{};
    static constexpr auto size = sizeof(data_);

public:
    SerializedData() = default;
    explicit SerializedData(const data_type &v) {
        serialize_impl(v);
    }

    void serialize_impl(const data_type &value) {
        if constexpr (is_pod) {
            oc_memcpy(addressof(data_), addressof(value), size);
        } else {
            
        }
    }

    void serialize(void *address) noexcept override {
        serialize_impl(*reinterpret_cast<data_type*>(address));
    }

    void deserialize(void *address) const noexcept override {
        oc_memcpy(address, addressof(data_), size);
    }
};

class RuntimeObject;

class Serializer {
public:
    using handle_t = const RuntimeObject *;
    using attr_map_t = std::map<ocarina::string_view, UP<Serializable>>;
    using object_map_t = std::map<handle_t, attr_map_t>;

private:
    object_map_t object_map_;

private:
    [[nodiscard]] attr_map_t &get_attr_map(handle_t object) noexcept {
        if (!object_map_.contains(object)) {
            object_map_.insert(make_pair(object, attr_map_t{}));
        }
        return object_map_.at(object);
    }

public:
    Serializer() = default;

    void erase_old_object(handle_t object) {
        object_map_.erase(object);
    }

    template<typename T>
    void serialize(handle_t old_obj, ocarina::string_view name, const T &value) {
        attr_map_t &attr_map = get_attr_map(old_obj);
        attr_map.insert(make_pair(name, make_unique<SerializedData<T>>(value)));
    }

    template<typename T>
    void deserialize(handle_t old_obj, ocarina::string_view name, T &value) {
        attr_map_t &attr_map = get_attr_map(old_obj);
        Serializable *serializable = attr_map.at(name).get();
        serializable->deserialize(address(value));
    }
};

}// namespace vision::inline hotfix