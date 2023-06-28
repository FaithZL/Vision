//
// Created by Zero on 2023/6/25.
//

#include "graph.h"
#include <stack>

namespace vision {

void RenderGraph::add_edge(const string &output, const string &input) noexcept {
    _edges.emplace_back(output, input);
}

void RenderGraph::mark_output(const string &output) noexcept {
    _output = Field(output);
}

RenderGraph::EdgeData RenderGraph::_find_edge(const RenderGraph::Field &dst) noexcept {
    for (const auto &edge : _edges) {
        if (dst.str() == edge.dst.str()) {
            return edge;
        }
    }
    return {};
}

void RenderGraph::_DFS_traverse(vision::RenderPass *pass) noexcept {
    for (const auto &input : pass->inputs()) {
        string dst = pass->name() + "." + input.name;
        EdgeData edge = _find_edge(dst);
        if (edge.empty()) {
            continue ;
        }
        RenderPass *output_pass = _pass_map[edge.src.pass()];
        _DFS_traverse(output_pass);
    }
    _pass_set.emplace(pass);
    int i = 0;
}

void RenderGraph::_simplification() noexcept {
    RenderPass *output_pass = _pass_map[_output.pass()];
    _DFS_traverse(output_pass);
}

void RenderGraph::_build_command_list() noexcept {

}

void RenderGraph::_build_graph() noexcept {
    _simplification();
    _build_command_list();
    _allocate_resource();
}

void RenderGraph::_allocate_resource() noexcept {
}

void RenderGraph::setup() noexcept {
    _build_graph();
}

void RenderGraph::compile() noexcept {
}

void RenderGraph::execute() noexcept {
}
}// namespace vision