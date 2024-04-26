//
// Created by Zero on 09/09/2022.
//

#pragma once

#include <utility>
#include "core/stl.h"
#include "UI/GUI.h"
#include "descriptions/node_desc.h"
#include "util/file_manager.h"

namespace ocarina {
class Device;
}

namespace vision {

class Pipeline;
class SpectrumImpl;
template<typename impl_t, typename desc_t>
class TObject;
using Spectrum = TObject<SpectrumImpl, SpectrumDesc>;
class FrameBuffer;

using namespace ocarina;

class Node : public Hashable, public GUI {
public:
    using Creator = Node *(const NodeDesc &);
    using Deleter = void(Node *);
    using Wrapper = shared_ptr<Node>;

protected:
    string name_;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override { return 0; }

public:
    Node() = default;
    explicit Node(const NodeDesc &desc) : name_(desc.name) {}
    [[nodiscard]] static Pipeline *pipeline() noexcept;
    [[nodiscard]] static Scene &scene() noexcept;
    [[nodiscard]] static fs::path scene_path() noexcept;
    [[nodiscard]] static Spectrum &spectrum() noexcept;
    [[nodiscard]] static FrameBuffer &frame_buffer() noexcept;
    [[nodiscard]] static Device &device() noexcept;
    virtual void prepare() noexcept {}
    [[nodiscard]] virtual string to_string() noexcept { return "node"; }
    [[nodiscard]] virtual string_view impl_type() const noexcept = 0;
    [[nodiscard]] virtual string_view category() const noexcept = 0;
    [[nodiscard]] string plugin_name() const noexcept {
        return ocarina::format("vision-{}-{}", category().data(), impl_type().data());
    }
    template<typename impl_t, typename Desc>
    [[nodiscard]] static SP<impl_t> load(const Desc &desc) {
        const DynamicModule *module = FileManager::instance().obtain_module(desc.plugin_name());
        auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
        auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
        SP<impl_t> ret = SP<impl_t>(dynamic_cast<impl_t *>(creator(desc)), deleter);
        OC_ERROR_IF(ret == nullptr, "error node load ", desc.name);
        return ret;
    }
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] string name() const noexcept { return name_; }
    void set_name(const string &name) noexcept { name_ = name; }
    ~Node() override = default;
};

template<typename impl_t, typename desc_t = impl_t::Desc>
class TObject {
public:
    using Impl = impl_t;
    using Desc = desc_t;

protected:
    SP<Impl> impl_;

public:
    string name;
    TObject() = default;
    TObject(SP<Impl> sp) : impl_(ocarina::move(sp)) {}
    explicit TObject(const desc_t &desc) : impl_(Node::load<Impl>(desc)) {}
    void init(const desc_t &desc) noexcept { impl_ = Node::load<Impl>(desc); }
    void init(SP<Impl> sp) { impl_ = ocarina::move(sp); }
    OC_MAKE_MEMBER_GETTER(impl, &)
    [[nodiscard]] operator bool() const noexcept { return impl_.get() != nullptr; }
    [[nodiscard]] const Impl *get() const noexcept { return impl_.get(); }
    [[nodiscard]] Impl *get() noexcept { return impl_.get(); }
    [[nodiscard]] const Impl *operator->() const noexcept { return impl_.get(); }
    [[nodiscard]] Impl *operator->() noexcept { return impl_.get(); }
    [[nodiscard]] const Impl &operator*() const noexcept { return *impl_.get(); }
    [[nodiscard]] Impl &operator*() noexcept { return *impl_.get(); }
    template<typename... Args>
    void reset(Args &&...args) noexcept { impl_.reset(OC_FORWARD(args)...); }
};

template<typename To, typename From, typename Desc>
requires std::is_base_of_v<From, To>
[[nodiscard]] auto dynamic_object_cast(TObject<From, Desc> object) noexcept {
    return TObject<To, Desc>(std::move(std::dynamic_pointer_cast<To>(object.impl())));
}

#define VS_MAKE_PLUGIN_NAME_FUNC                                                                 \
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; } \
    [[nodiscard]] string_view category() const noexcept override { return VISION_CATEGORY; }

}// namespace vision

#define VS_MAKE_CLASS_CREATOR(Class)                                                        \
    VS_EXPORT_API Class *create(const vision::NodeDesc &desc) {                             \
        return ocarina::new_with_allocator<Class>(dynamic_cast<const Class::Desc &>(desc)); \
    }                                                                                       \
    OC_EXPORT_API void destroy(Class *obj) {                                                \
        ocarina::delete_with_allocator(obj);                                                \
    }
