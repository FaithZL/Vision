//
// Created by Zero on 2023/6/25.
//

#include "graph.h"

namespace vision {

void RenderGraph::add_edge(const string &output, const string &input) noexcept {
    _edges.emplace_back(output, input);
}

void RenderGraph::mark_output(const string &output) noexcept {
    _output = Field(output);
}

void RenderGraph::_simplification() noexcept {
    RenderPass *output_pass = _pass_map[_output.pass()];


    std::list<FieldPair> connections;
    auto match_input = [&](const string &input) {
        for (auto iter = _edges.begin(); iter != _edges.end(); ++iter) {
            if (iter->second.pass() == input) {
                return iter;
            }
        }
        return _edges.end();
    };
    string input_pass = _output.pass();
    auto iter = _edges.begin();
    while ((iter = match_input(input_pass)) != _edges.end()) {
        input_pass = iter->first.pass();
        connections.push_back(*iter);
    }
    _edges = ocarina::move(connections);
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