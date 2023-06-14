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

class Pipeline;
class Spectrum;

using namespace ocarina;

class Node : public Hashable {
public:
    using Creator = Node *(const NodeDesc &);
    using Deleter = void(Node *);
    using Wrapper = unique_ptr<Node, Node::Deleter *>;

protected:
    string _name;

public:
    Node() = default;
    explicit Node(const NodeDesc &desc) : _name(desc.name) {}
    [[nodiscard]] static Pipeline *pipeline() noexcept;
    [[nodiscard]] static Scene &scene() noexcept;
    [[nodiscard]] Spectrum &spectrum() noexcept;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override { return 0; }
    [[nodiscard]] const Spectrum &spectrum() const noexcept;
    [[nodiscard]] Device &device() noexcept;
    virtual void prepare() noexcept {}
    [[nodiscard]] string name() const noexcept { return _name; }
    virtual ~Node() = default;
};

}// namespace vision

#define VS_MAKE_CLASS_CREATOR(Class)                                                        \
    VS_EXPORT_API Class *create(const vision::NodeDesc &desc) {                             \
        return ocarina::new_with_allocator<Class>(dynamic_cast<const Class::Desc &>(desc)); \
    }                                                                                       \
    OC_EXPORT_API void destroy(Class *obj) {                                                \
        ocarina::delete_with_allocator(obj);                                                \
    }
