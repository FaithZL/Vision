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
class FrameBuffer;

using namespace ocarina;

class Node : public Hashable {
public:
    using Creator = Node *(const NodeDesc &);
    using Deleter = void(Node *);
    using Wrapper = shared_ptr<Node>;

protected:
    string _name;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override { return 0; }

public:
    Node() = default;
    explicit Node(const NodeDesc &desc) : _name(desc.name) {}
    [[nodiscard]] static Pipeline *pipeline() noexcept;
    [[nodiscard]] static Scene &scene() noexcept;
    [[nodiscard]] static fs::path scene_path() noexcept;
    [[nodiscard]] static Spectrum &spectrum() noexcept;
    [[nodiscard]] static FrameBuffer &frame_buffer() noexcept;
    [[nodiscard]] static Device &device() noexcept;
    virtual void prepare() noexcept {}
    [[nodiscard]] virtual string to_string() noexcept { return "node"; }
    [[nodiscard]] virtual string_view impl_type() const noexcept = 0;
    [[nodiscard]] string name() const noexcept { return _name; }
    void set_name(const string &name) noexcept { _name = name; }
    virtual ~Node() = default;
};

template<typename T>
struct Wrap {
    string name;
    SP<T> object;
};

}// namespace vision

#define VS_MAKE_CLASS_CREATOR(Class)                                                        \
    VS_EXPORT_API Class *create(const vision::NodeDesc &desc) {                             \
        return ocarina::new_with_allocator<Class>(dynamic_cast<const Class::Desc &>(desc)); \
    }                                                                                       \
    OC_EXPORT_API void destroy(Class *obj) {                                                \
        ocarina::delete_with_allocator(obj);                                                \
    }
