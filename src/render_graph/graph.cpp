//
// Created by Zero on 2023/6/25.
//

#include "graph.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"

namespace vision {

void RenderGraph::add_edge(const string &output, const string &input) noexcept {
    edges_.emplace_back(output, input);
}

void RenderGraph::mark_output(const string &output) noexcept {
    output_ = Field(output);
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
    for (const auto &edge : edges_) {
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
        simple_edges_.push_back(edge);
        RenderPass *output_pass = pass_map_[edge.src.pass()].get();
        DFS_traverse(output_pass);
    }
    pass_list_.push_back(pass);
}

void RenderGraph::_simplification() noexcept {
    RenderPass *output_pass = pass_map_[output_.pass()].get();
    DFS_traverse(output_pass);
}

void RenderGraph::_build_graph() noexcept {
    _simplification();
    _allocate_resource();
}

void RenderGraph::_allocate_resource() noexcept {
    for (const auto &edge : simple_edges_) {
        const Field &src = edge.src;
        const Field &dst = edge.dst;
        Buffer<float4> buffer = device().create_buffer<float4>(pixel_num());
        UP<RenderResource> render_resource = make_unique<TResource<Buffer<float4>>>(ocarina::move(buffer));
        RenderPass *src_pass = pass_map_[src.pass()].get();
        src_pass->set_resource(src.channel(), *render_resource);
        RenderPass *dst_pass = pass_map_[dst.pass()].get();
        dst_pass->set_resource(dst.channel(), *render_resource);
        resources_.push_back(ocarina::move(render_resource));
    }
    Buffer<float4> buffer = device().create_buffer<float4>(pixel_num());
    UP<RenderResource> render_resource = make_unique<TResource<Buffer<float4>>>(ocarina::move(buffer));
    RenderPass *pass = pass_map_[output_.pass()].get();
    pass->set_resource(output_.channel(), *render_resource);
    resources_.push_back(ocarina::move(render_resource));
}

void RenderGraph::setup() noexcept {
    _build_graph();
}

void RenderGraph::compile() noexcept {
    for (const auto &pass : pass_list_.commands()) {
        pass->compile();
    }
}

CommandList RenderGraph::dispatch() noexcept {
    CommandList ret;
    for (const auto &pass : pass_list_.commands()) {
        ret.push_back(pass->dispatch());
    }
    return ret;
}

}// namespace vision