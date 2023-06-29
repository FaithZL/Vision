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
    using Desc = RenderPassDesc;

private:
    std::map<string, RenderResource *> _res_map;

public:
    RenderPass() = default;
    explicit RenderPass(const RenderPassDesc &desc) : Node(desc) {}
    [[nodiscard]] const RenderResource *get_resource(const string &name) const noexcept {
        if (_res_map.find(name) == _res_map.cend()) {
            return nullptr;
        }
        return _res_map.at(name);
    }
    template<typename T>
    void set_resource(const string &name, const T &res) noexcept {
        _res_map.insert(std::make_pair(name, *res));
    }
    [[nodiscard]] virtual ChannelList inputs() const noexcept { return {}; }
    [[nodiscard]] virtual ChannelList outputs() const noexcept { return {}; }
    virtual void compile() noexcept {}
    virtual Command *dispatch() noexcept { return nullptr; }
    [[nodiscard]] static RenderPass *create(const string &name, const ParameterSet &ps = {}) noexcept;
};

}// namespace vision