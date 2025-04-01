//
// Created by Zero on 09/09/2022.
//

#pragma once

#include <utility>
#include "core/stl.h"
#include "UI/GUI.h"
#include "GUI/widgets.h"
#include "core/node_desc.h"
#include "util/file_manager.h"
#include "hotfix/hotfix.h"

namespace ocarina {
class Device;
}
namespace vision {
class Pipeline;
class Spectrum;
template<typename impl_t, typename desc_t>
class TObject;
using TSpectrum = TObject<Spectrum, SpectrumDesc>;
class FrameBuffer;
using namespace ocarina;

class Node : public RuntimeObject, public GUI, public Hashable {
protected:
    string name_;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override { return 0; }

public:
    Node() = default;
    VS_HOTFIX_MAKE_RESTORE(RuntimeObject, name_)
    explicit Node(const NodeDesc &desc) : name_(desc.name) {}
    [[nodiscard]] const char *class_name() const noexcept override {
        return Hashable::class_name();
    }
    [[nodiscard]] static Pipeline *pipeline() noexcept;
    [[nodiscard]] static Scene &scene() noexcept;
    [[nodiscard]] static fs::path scene_path() noexcept;
    [[nodiscard]] static TSpectrum &spectrum() noexcept;
    [[nodiscard]] static FrameBuffer &frame_buffer() noexcept;
    [[nodiscard]] static Device &device() noexcept;
    virtual void prepare() noexcept {}
    virtual void initialize_(const NodeDesc &node_desc) noexcept {}
    virtual void upload_immediately() noexcept {}
    [[nodiscard]] virtual string to_string() noexcept { return "node"; }
    [[nodiscard]] virtual string_view impl_type() const noexcept = 0;
    [[nodiscard]] virtual string_view category() const noexcept = 0;
    [[nodiscard]] string plugin_name() const noexcept {
        return ocarina::format("vision-{}-{}", category().data(), impl_type().data());
    }

    template<typename impl_t, typename Desc>
    [[nodiscard]] static SP<impl_t> create_shared(const Desc &desc);

    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] string name() const noexcept { return name_; }
    void set_name(const string &name) noexcept { name_ = name; }
    ~Node() override = default;
};

#define VS_CAST_DESC const Desc &desc = static_cast<const Desc &>(node_desc);

class INodeConstructor {
public:
    using Deleter = void(Node *);

public:
    INodeConstructor() = default;
    [[nodiscard]] virtual Node *construct_impl(const NodeDesc *desc) const = 0;
    [[nodiscard]] virtual SP<Node> construct_shared_impl(const NodeDesc *desc) const = 0;
    [[nodiscard]] virtual UP<Node, Deleter *> construct_unique_impl(const NodeDesc *desc) const = 0;
    template<typename T = Node>
    [[nodiscard]] T *construct(const NodeDesc *desc) const {
        return static_cast<T *>(construct_impl(desc));
    }
    template<typename T = Node>
    [[nodiscard]] SP<T> construct_shared(const NodeDesc *desc) const noexcept {
        return std::static_pointer_cast<T>(construct_shared_impl(desc));
    }
    template<typename T = Node>
    [[nodiscard]] UP<T, Deleter *> construct_unique(const NodeDesc *desc) const noexcept {
        return ocarina::static_unique_pointer_cast<T>(construct_unique_impl(desc));
    }
    static void destroy(Node *obj) {
        ocarina::delete_with_allocator(obj);
    }
};

template<typename T>
requires std::derived_from<T, Node>
class NodeConstructor : INodeConstructor {
public:
    [[nodiscard]] Node *construct_impl(const NodeDesc *desc) const override {
        return ocarina::new_with_allocator<T>(*dynamic_cast<const T::Desc *>(desc));
    }
    [[nodiscard]] SP<Node> construct_shared_impl(const vision::NodeDesc *desc) const override {
        return SP<T>(static_cast<T *>(construct_impl(desc)), destroy);
    }
    [[nodiscard]] UP<Node, Deleter *> construct_unique_impl(const vision::NodeDesc *desc) const override {
        return UP<T, Deleter *>(static_cast<T *>(construct_impl(desc)), destroy);
    }
};

template<typename impl_t, typename Desc>
SP<impl_t> Node::create_shared(const Desc &desc) {
    const DynamicModule *module = FileManager::instance().obtain_module(desc.plugin_name());
    using Constructor = INodeConstructor *();
    Constructor *constructor = module->function<Constructor *>("constructor");
    SP<impl_t> ret = constructor()->construct_shared<impl_t>(&desc);
    ret->initialize_(desc);
    OC_ERROR_IF(ret == nullptr, "error node load ", desc.name);
    return ret;
}

#define VS_MAKE_PLUGIN_NAME_FUNC                                                                 \
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; } \
    [[nodiscard]] string_view category() const noexcept override { return VISION_CATEGORY; }

template<typename impl_t, typename desc_t = typename impl_t::Desc>
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
    explicit TObject(const desc_t &desc) : impl_(Node::create_shared<Impl>(desc)) {}
    void init(const desc_t &desc) noexcept { impl_ = Node::create_shared<Impl>(desc); }
    void init(SP<Impl> sp) { impl_ = ocarina::move(sp); }

    template<class U>
    requires std::is_base_of_v<Impl, U>
    TObject(const TObject<U, Desc> &other) {
        impl_ = other.impl();
        name = other.name;
    }

    template<class U>
    requires std::is_base_of_v<Impl, U>
    TObject &operator=(const TObject<U, Desc> &other) {
        impl_ = other.impl();
        name = other.name;
        return *this;
    }

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

template<typename To, typename From, typename Desc>
requires std::is_base_of_v<From, To>
[[nodiscard]] auto static_object_cast(TObject<From, Desc> object) noexcept {
    return TObject<To, Desc>(std::move(std::static_pointer_cast<To>(object.impl())));
}

template<typename impl_t, typename desc_t = impl_t::Desc>
class TObjectUI : public TObject<impl_t, desc_t>, public GUI {
public:
    using TObject<impl_t, desc_t>::TObject;
    using Super = TObject<impl_t, desc_t>;

protected:
    int current_item_{0};
    vector<const char *> extract_name_list() noexcept {
        static vector<string> name_list = [&]() {
            vector<string> ret;
            string pattern = ocarina::format("vision-{}-.*\\.dll", Super::impl()->category().data());
            for (const auto &entry : fs::directory_iterator(".")) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (std::regex_search(filename, std::regex(pattern))) {
                        ret.push_back(filename);
                    }
                }
            }
            return ret;
        }();
        vector<const char *> ret;
        for (const string &fn : name_list) {
            ret.push_back(fn.c_str());
        }
        return ret;
    }

public:
    bool has_changed() noexcept override {
        return Super::impl()->has_changed();
    }
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        return widgets->use_folding_header(Super::impl()->category().data(), [&] {
            //            auto names = extract_name_list();
            //            for (int i = 0; i < names.size(); ++i) {
            //                if (Super::impl()->plugin_name() + ".dll" == names[i]) {
            //                    current_item_ = i;
            //                    break;
            //                }
            //            }
            //            widgets->combo(Super::impl()->category().data(), &current_item_, names);
            Super::impl()->render_UI(widgets);
        });
    }
    void reset_status() noexcept override {
        Super::impl()->reset_status();
    }
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        Super::impl()->render_sub_UI(widgets);
    }
};

}// namespace vision

#define VS_MAKE_CLASS_CREATOR(Class)                              \
    VS_EXPORT_API vision::NodeConstructor<Class> *constructor() { \
        static vision::NodeConstructor<Class> ret;                \
        return &ret;                                              \
    }

#define VS_MAKE_CLASS_CREATOR_HOTFIX(NS, Class) \
    VS_REGISTER_HOTFIX(NS, Class)               \
    VS_MAKE_CLASS_CREATOR(NS::Class)