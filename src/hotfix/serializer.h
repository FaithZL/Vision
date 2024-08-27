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

    template<typename T>
    void deserialize(string_view field_name, T &value);

    virtual void print(int &indent) const noexcept = 0;
};

namespace {
string tab_string(int indent) {
    string ret;
    for (int i = 0; i < indent; ++i) {
        ret += "    ";
    }
    return ret;
}
}// namespace

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

    static SP<SerializedData<T>> apply(T value) noexcept {
        SP<SerializedData<T>> ret = make_shared<SerializedData<T>>();
        if constexpr (is_pod) {
            ret->data_ = value;
        } else if constexpr (is_runtime_object) {
            value->serialize(ret);
        }
        return ret;
    }

    void print(int &indent) const noexcept override {
        if constexpr (is_runtime_object) {
            cout << tab_string(indent) << endl;
            indent++;
            for (const auto &it : data_) {
                cout << tab_string(indent) << "" << it.first << " = ";
                it.second->print(indent);
            }
            indent--;
        } else if constexpr (is_pod) {
            cout << "" << data_ << endl;
        }
    }

    void serialize_impl(std::string_view field_name, SP<Serializable> serializable) override {
        if constexpr (is_runtime_object) {
            data_.insert(make_pair(field_name, serializable));
        }
    }
};

template<typename T>
void Serializable::serialize(std::string_view field_name, T value) {
    serialize_impl(field_name, SerializedData<T>::apply(value));
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

    void clear() noexcept { object_map_.clear(); }

    void print() noexcept {
        int indent = 0;
        for (const auto &item : object_map_) {
            cout << "object type = " << type_string<decltype(item.first)>();
            item.second->print(indent);
        }
    }

    template<typename T>
    void serialize(T &&object) noexcept {
        auto ptr = raw_ptr(OC_FORWARD(object));
        object_map_.insert(make_pair(ptr, SerializedData<T>::apply(OC_FORWARD(object))));
    }

    template<typename T>
    void deserialize(T &&object) noexcept {

    }
};

}// namespace vision::inline hotfix