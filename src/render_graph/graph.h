//
// Created by Zero on 2023/6/25.
//

#pragma once

#include "rhi/common.h"
#include "resource.h"
#include "pass.h"

namespace vision {
using namespace ocarina;
/**
 * 1. add passes
 * 2. build connections
 * 3. eliminate redundant passes and compile
 * 4. allocate resources
 * 5. execute
 */
class RenderGraph {
private:
    unordered_map<string, RenderPass *> _pass_map;
    vector<UP<RenderResource>> _resources;
    vector<pair<string, string>> _connections;
    string _output;

private:
    vector<RenderPass *> _command_list;

    void _build_graph() noexcept;
    void _allocate_resource() noexcept;

public:
    void add_pass(RenderPass *pass, string name) noexcept {
        _pass_map.insert(std::make_pair(ocarina::move(name), pass));
    }
    void clear_resources() noexcept {
        _resources.clear();
    }
    void clear_passes() noexcept {
        _pass_map.clear();
    }
    void clear_connections() noexcept {
        _connections.clear();
    }
    void build_connection(const string &output, const string &input) noexcept;
    void mark_output(const string &output) noexcept;
    void setup() noexcept;
    void compile() noexcept;
    void execute() noexcept;
};

}// namespace vision