//
// Created by Zero on 09/09/2022.
//

#pragma once

#include <utility>
#include "core/stl.h"
#include "descriptions/node_desc.h"

namespace ocarina {
class Device;
}

namespace vision {

class RenderPipeline;
class Spectrum;

using namespace ocarina;

class Object : public Hashable {
public:
    using Creator = Object *(const ObjectDesc &);
    using Deleter = void(Object *);
    using Wrapper = unique_ptr<Object, Object::Deleter *>;

public:
    virtual void prepare() noexcept {}
    [[nodiscard]] uint64_t _compute_hash() const noexcept override { return 0; }
    virtual ~Object() = default;
};

class Node : public Object {
protected:
    string _name;
    mutable Scene *_scene{nullptr};
    uint _type_index{InvalidUI32};

public:
    Node() = default;
    explicit Node(const ObjectDesc &desc) : _name(desc.name), _scene(desc.scene) {}
    [[nodiscard]] RenderPipeline *render_pipeline() noexcept;
    [[nodiscard]] const RenderPipeline *render_pipeline() const noexcept;
    [[nodiscard]] Spectrum &spectrum() noexcept;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override { return 0; }
    [[nodiscard]] uint type_index() const noexcept { return _type_index; }
    void set_type_index(uint val) noexcept { _type_index = val; }
    [[nodiscard]] const Spectrum &spectrum() const noexcept;
    [[nodiscard]] Device &device() noexcept;
    void prepare() noexcept override {}
    [[nodiscard]] string name() const noexcept { return _name; }
};
}// namespace vision

#define VS_MAKE_CLASS_CREATOR(Class)                                                        \
    VS_EXPORT_API Class *create(const vision::ObjectDesc &desc) {                           \
        return ocarina::new_with_allocator<Class>(dynamic_cast<const Class::Desc &>(desc)); \
    }                                                                                       \
    OC_EXPORT_API void destroy(Class *obj) {                                                \
        ocarina::delete_with_allocator(obj);                                                \
    }
