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
    mutable Scene *_scene{nullptr};

public:
    Node() = default;
    explicit Node(const NodeDesc &desc) : _name(desc.name), _scene(desc.scene) {}
    [[nodiscard]] Pipeline *pipeline() noexcept;
    [[nodiscard]] const Pipeline *pipeline() const noexcept;
    [[nodiscard]] Spectrum &spectrum() noexcept;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override { return 0; }
    [[nodiscard]] const Spectrum &spectrum() const noexcept;
    [[nodiscard]] Device &device() noexcept;
    virtual void prepare() noexcept {}
    [[nodiscard]] string name() const noexcept { return _name; }
    virtual ~Node() = default;
};

class NodeMgr {
public:
    using Container = std::list<Node::Wrapper>;
    using Iterator = Container::iterator;
private:
    std::list<Node::Wrapper> _all_nodes;

private:
    NodeMgr() = default;
    NodeMgr(const NodeMgr &) = delete;
    NodeMgr(NodeMgr &&) = delete;
    NodeMgr operator=(const NodeMgr &) = delete;
    NodeMgr operator=(NodeMgr &&) = delete;
    static NodeMgr *s_node_loader;

public:
    [[nodiscard]] static NodeMgr &instance() noexcept;
    static void destroy_instance() noexcept;
    Container::iterator remove(Node *node) noexcept;
};

}// namespace vision

#define VS_MAKE_CLASS_CREATOR(Class)                                                        \
    VS_EXPORT_API Class *create(const vision::NodeDesc &desc) {                             \
        return ocarina::new_with_allocator<Class>(dynamic_cast<const Class::Desc &>(desc)); \
    }                                                                                       \
    OC_EXPORT_API void destroy(Class *obj) {                                                \
        ocarina::delete_with_allocator(obj);                                                \
    }
