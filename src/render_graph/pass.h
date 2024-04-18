//
// Created by Zero on 2023/6/24.
//

#pragma once

#include "rhi/common.h"
#include "base/node.h"
#include "resource.h"

namespace vision {

using namespace ocarina;

struct ChannelDesc {
    string name;
    string desc;
    bool optional;
    ResourceFormat format;
};

using ChannelList = vector<ChannelDesc>;

class RenderPass : public Node {
public:
    using Desc = PassDesc;

private:
    std::map<string, const RenderResource *> res_map_;
    bool recompile_{false};

public:
    RenderPass() = default;
    explicit RenderPass(const PassDesc &desc) : Node(desc) {}
    [[nodiscard]] const RenderResource *get_resource(const string &name) const noexcept {
        if (res_map_.find(name) == res_map_.cend()) {
            return nullptr;
        }
        return res_map_.at(name);
    }
    OC_MAKE_MEMBER_GETTER_(recompile, )
    template<typename T>
    void set_resource(const string &name, const T &res) noexcept {
        res_map_.insert(std::make_pair(name, &res));
    }
    template<typename T>
    [[nodiscard]] const T &res(const string &name) const noexcept {
        const RenderResource *render_resource = get_resource(name);
        const T *rhi_res = dynamic_cast<const T *>(render_resource->rhi_resource());
        return *rhi_res;
    }
    [[nodiscard]] virtual ChannelList inputs() const noexcept { return {}; }
    [[nodiscard]] virtual ChannelList outputs() const noexcept { return {}; }
    virtual void set_parameter(const ParameterSet &ps) noexcept {}
    virtual void compile() noexcept {}
    virtual Command *dispatch() noexcept { return nullptr; }
    [[nodiscard]] static SP<RenderPass> create(const string &name, const ParameterSet &ps = DataWrap::object()) noexcept;
};

}// namespace vision