//
// Created by Zero on 09/09/2022.
//

#pragma once

#include <utility>
#include "core/stl.h"
#include "descriptions/node_desc.h"

namespace vision {

class RenderPipeline;

class Node {
protected:
    ocarina::string _name;
    mutable Scene *_scene{nullptr};

public:
    using Creator = Node *(const NodeDesc &);
    using Deleter = void(Node *);
    using Wrapper = ocarina::unique_ptr<Node, Node::Deleter *>;

public:
    Node() = default;
    explicit Node(const NodeDesc &desc) : _name(desc.name), _scene(desc.scene) {}
    virtual void prepare(RenderPipeline *rp) noexcept {}
    virtual ~Node() = default;
    [[nodiscard]] ocarina::string name() const noexcept { return _name; }
};
}// namespace vision

#define VS_MAKE_CLASS_CREATOR(Class)                                                        \
    VS_EXPORT_API Class *create(const vision::NodeDesc &desc) {                             \
        return ocarina::new_with_allocator<Class>(dynamic_cast<const Class::Desc &>(desc)); \
    }                                                                                       \
    OC_EXPORT_API void destroy(Class *obj) {                                                \
        ocarina::delete_with_allocator(obj);                                                \
    }
