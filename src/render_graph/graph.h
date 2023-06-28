//
// Created by Zero on 2023/6/25.
//

#pragma once

#include "rhi/common.h"
#include "resource.h"
#include "pass.h"

namespace vision {

namespace detail {
[[nodiscard]] pair<string, string> pass_channel(const string &fullname) noexcept {
    string pass;
    string channel;
    size_t dot = fullname.find_last_of('.');
    pass = fullname.substr(0, dot);
    channel = fullname.substr(dot + 1);
    return std::make_pair(pass, channel);
}
}// namespace detail

using namespace ocarina;
/**
 * 1. add passes
 * 2. build connections
 * 3. eliminate redundant passes and compile
 * 4. allocate resources
 * 5. execute
 */
class RenderGraph {
public:
    using str_pair = pair<string, string>;
    class Field {
    private:
        str_pair _pair;

    public:
        Field() = default;
        Field(const string &str)
            : _pair(detail::pass_channel(str)) {}
        [[nodiscard]] string pass() const noexcept { return _pair.first; }
        [[nodiscard]] string channel() const noexcept { return _pair.second; }
        [[nodiscard]] string str() const noexcept { return pass() + "." + channel(); }
    };
    using FieldPair = pair<Field, Field>;

private:
    unordered_map<string, RenderPass *> _pass_map;
    vector<UP<RenderResource>> _resources;
    std::list<FieldPair> _edges;
    Field _output;

private:
    vector<RenderPass *> _command_list;

    void _build_graph() noexcept;
    void _simplification() noexcept;
    void _build_command_list() noexcept;
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
        _edges.clear();
    }
    void add_edge(const string &output, const string &input) noexcept;
    void mark_output(const string &output) noexcept;
    void setup() noexcept;
    void compile() noexcept;
    void execute() noexcept;
};

}// namespace vision