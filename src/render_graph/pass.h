//
// Created by Zero on 2023/6/24.
//

#pragma once

#include "rhi/common.h"
#include "base/node.h"
#include "resource.h"

namespace vision {

using namespace ocarina;

class RenderPass : public Node {
private:
    std::map<string, RenderResource *> _res_map;

public:
    RenderPass() = default;
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

    virtual void setup() noexcept {}
    virtual void compile() noexcept {}
    virtual void execute() noexcept {}
};

}// namespace vision