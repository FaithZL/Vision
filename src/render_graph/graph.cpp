//
// Created by Zero on 2023/6/25.
//

#include "graph.h"

namespace vision {

void RenderGraph::build_connection(const string &output, const string &input) noexcept {
    _connections.emplace_back(output, input);
}

void RenderGraph::mark_output(const string &output) noexcept {
    _output = output;
}

void RenderGraph::_build_graph() noexcept {

}

void RenderGraph::_allocate_resource() noexcept {

}

void RenderGraph::setup() noexcept {
}
void RenderGraph::compile() noexcept {
}
void RenderGraph::execute() noexcept {
}
}// namespace vision