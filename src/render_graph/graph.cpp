//
// Created by Zero on 2023/6/25.
//

#include "graph.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"

namespace vision {

void RenderGraph::add_edge(const string &output, const string &input) noexcept {
    _edges.emplace_back(output, input);
}

void RenderGraph::mark_output(const string &output) noexcept {
    _output = Field(output);
}

Pipeline *RenderGraph::pipeline() noexcept {
    return Global::instance().pipeline();
}

Device &RenderGraph::device() noexcept {
    return pipeline()->device();
}

uint2 RenderGraph::resolution() noexcept {
    return pipeline()->resolution();
}

uint RenderGraph::pixel_num() noexcept {
    return resolution().x * resolution().y;
}

RenderGraph::EdgeData RenderGraph::_find_edge(const RenderGraph::Field &dst) noexcept {
    for (const auto &edge : _edges) {
        if (dst.str() == edge.dst.str()) {
            return edge;
        }
    }
    return {};
}

void RenderGraph::DFS_traverse(vision::RenderPass *pass) noexcept {
    for (const auto &input : pass->inputs()) {
        string dst = pass->name() + "." + input.name;
        EdgeData edge = _find_edge(dst);
        if (edge.empty()) {
            continue;
        }
        _simple_edges.push_back(edge);
        RenderPass *output_pass = _pass_map[edge.src.pass()];
        DFS_traverse(output_pass);
    }
    _pass_list.push_back(pass);
}

void RenderGraph::_simplification() noexcept {
    RenderPass *output_pass = _pass_map[_output.pass()];
    DFS_traverse(output_pass);
}

void RenderGraph::_build_graph() noexcept {
    _simplification();
    _allocate_resource();
}

void RenderGraph::_allocate_resource() noexcept {
    for (const auto &edge : _simple_edges) {
        const Field &src = edge.src;
        const Field &dst = edge.dst;
        Buffer<float4> buffer = device().create_buffer<float4>(pixel_num());
        UP<RenderResource> render_resource = make_unique<TResource<Buffer<float4>>>(ocarina::move(buffer));
        RenderPass *src_pass = _pass_map[src.pass()];
        src_pass->set_resource(src.channel(), *render_resource);
        RenderPass *dst_pass = _pass_map[dst.pass()];
        dst_pass->set_resource(dst.channel(), *render_resource);
        _resources.push_back(ocarina::move(render_resource));
    }
}

void RenderGraph::setup() noexcept {
    _build_graph();
}

void RenderGraph::compile() noexcept {
    for (const auto &pass : _pass_list.commands()) {
        pass->compile();
    }
}

vector<Command*> RenderGraph::dispatch() noexcept {
    vector<Command*> ret;
    for (const auto &pass : _pass_list.commands()) {
        ret.push_back(pass->dispatch());
    }
    return ret;
}

}// namespace vision