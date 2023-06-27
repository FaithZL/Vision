//
// Created by Zero on 2023/6/25.
//

#include "graph.h"

namespace vision {

void RenderGraph::build_connection(const string &output, const string &input) noexcept {
    _connections.emplace_back(output, input);
}

void RenderGraph::mark_output(const string &output) noexcept {
    _output = Field(output);
}

void RenderGraph::_simplification_connections() noexcept {
    std::list<FieldPair> connections;
    auto match_input = [&](const string &input) {
        for (auto iter = _connections.begin(); iter != _connections.end(); ++iter) {
            if (iter->second.pass() == input) {
                return iter;
            }
        }
        return _connections.end();
    };
    string input_pass = _output.pass();
    auto iter = _connections.begin();
    while ((iter = match_input(input_pass)) != _connections.end()) {
        input_pass = iter->first.pass();
        connections.push_back(*iter);
    }
    _connections = ocarina::move(connections);
}

void RenderGraph::_build_command_list() noexcept {
    std::for_each(_connections.rbegin(), _connections.rend(), [&](FieldPair &field_pair){
        _command_list.push_back(_pass_map[field_pair.first.pass()]);
    });
    int i = 0;
}

void RenderGraph::_build_graph() noexcept {
    _simplification_connections();
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