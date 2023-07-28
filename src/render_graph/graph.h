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
        Field(const string &pass, const string &field)
            : _pair(pass, field) {}
        Field(const string &str)
            : _pair(detail::pass_channel(str)) {}
        [[nodiscard]] string pass() const noexcept { return _pair.first; }
        [[nodiscard]] string channel() const noexcept { return _pair.second; }
        [[nodiscard]] string str() const noexcept { return pass() + "." + channel(); }
        [[nodiscard]] bool empty() const noexcept { return pass().empty() || channel().empty(); }
    };

    struct EdgeData {
        Field src;
        Field dst;
        [[nodiscard]] bool empty() const noexcept { return src.empty() || dst.empty(); }
    };

    struct PassList {
    private:
        unordered_set<RenderPass *> _pass_set;
        vector<RenderPass *> _commands;

    public:
        OC_MAKE_MEMBER_GETTER(commands, &)
        void push_back(RenderPass *pass) noexcept {
            if (_pass_set.contains(pass)) {
                return;
            }
            _pass_set.emplace(pass);
            _commands.push_back(pass);
        }
    };

private:
    unordered_map<string, SP<RenderPass>> _pass_map;
    vector<UP<RenderResource>> _resources;
    std::list<EdgeData> _edges;
    Field _output;

private:
    PassList _pass_list;
    std::list<EdgeData> _simple_edges;

    void _build_graph() noexcept;
    void _simplification() noexcept;
    void _allocate_resource() noexcept;
    void DFS_traverse(RenderPass *pass) noexcept;
    [[nodiscard]] EdgeData _find_edge(const Field &dst) noexcept;

public:
    void add_pass(SP<RenderPass> pass, string name) noexcept {
        pass->set_name(name);
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
    template<typename T = Buffer<float4>>
    [[nodiscard]] const T &output_buffer() const noexcept {
        SP<RenderPass> pass = _pass_map.at(_output.pass());
        return pass->res<T>(_output.channel());
    }
    void add_edge(const string &output, const string &input) noexcept;
    void mark_output(const string &output) noexcept;
    [[nodiscard]] static Pipeline *pipeline() noexcept;
    [[nodiscard]] static Device &device() noexcept;
    [[nodiscard]] static uint2 resolution() noexcept;
    [[nodiscard]] static uint pixel_num() noexcept;
    void setup() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch() noexcept;
};

}// namespace vision