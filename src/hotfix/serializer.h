//
// Created by Zero on 2024/7/22.
//

#pragma once

#include "math/basic_types.h"
#include "core/stl.h"
#include "object.h"

namespace vision::inline hotfix {

template<typename T>
class SerializedData;

using namespace ocarina;
class Serializable {
public:
    virtual ~Serializable() = default;

    template<typename T>
    void serialize(string_view field_name, T value);

    virtual void serialize_impl(string_view field_name, SP<Serializable> serializable) = 0;
};

template<typename T>
class SerializedData : public Serializable {
public:
    using attr_map_t = std::map<ocarina::string_view, SP<Serializable>>;
    using raw_type = std::remove_cvref_t<T>;
    static constexpr bool is_pod = std::is_trivially_copyable_v<raw_type> && !std::is_pointer_v<raw_type>;
    static constexpr bool is_runtime_object = std::derived_from<std::remove_pointer_t<ptr_t<raw_type>>, RuntimeObject>;

    static_assert(is_pod || is_runtime_object);

private:
    static constexpr auto type_deduce() noexcept {
        if constexpr (is_pod) {
            return T{};
        } else if constexpr (is_runtime_object) {
            return attr_map_t{};
        }
    }

private:
    using data_type = decltype(type_deduce());
    data_type data_{};

public:
    SerializedData() = default;

    static SP<SerializedData<T>> create(T value) noexcept {
        SP<SerializedData<T>> ret = make_shared<SerializedData<T>>();
        if constexpr (is_pod) {

        } else if constexpr (is_runtime_object) {
        }
        return ret;
    }

    void serialize_impl(std::string_view field_name, SP<Serializable> serializable) override {
    }
};

template<typename T>
void Serializable::serialize(std::string_view field_name, T value) {
    serialize_impl(field_name, SerializedData<T>::create(value));
}

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
    [[nodiscard]] SP<Serializable> get_serialized_data(T &&old_obj) noexcept {
        if (!object_map_.contains(old_obj)) {
            object_map_.insert(make_pair(old_obj, SerializedData<T>::create(OC_FORWARD(old_obj))));
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