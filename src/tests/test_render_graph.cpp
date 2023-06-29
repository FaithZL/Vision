//
// Created by Zero on 2023/6/27.
//

#include <math/box.h>
#include "core/stl.h"
#include "rhi/common.h"
#include "render_graph/graph.h"

using namespace vision;
using namespace ocarina;

namespace vision {

class RTPass : public RenderPass {
private:
    static constexpr auto kOutput = "radiance";
    static constexpr auto kNormal = "normal";
    ChannelList _input = {};
    ChannelList _output = {
        {"radiance", "radiance", false, ResourceFormat::FLOAT4},
        {"normal", "normal", false, ResourceFormat::FLOAT4},
    };

public:
    [[nodiscard]] ChannelList inputs() const noexcept override { return _input; }
    [[nodiscard]] ChannelList outputs() const noexcept override { return _output; }
};

class AccumulatePass : public RenderPass {
private:
    static constexpr auto kInput = "input";
    static constexpr auto kOutput = "output";
    ChannelList _input = {
        {"in_radiance", "radiance", false, ResourceFormat::FLOAT4},
        {"in_normal", "normal", false, ResourceFormat::FLOAT4},
    };
    ChannelList _output = {
        {"radiance", "radiance", false, ResourceFormat::FLOAT4},
        {"normal", "normal", false, ResourceFormat::FLOAT4},
    };

public:
    [[nodiscard]] ChannelList inputs() const noexcept override { return _input; }
    [[nodiscard]] ChannelList outputs() const noexcept override { return _output; }
};

class DenoisePass : public RenderPass {
private:
    static constexpr auto kRadiance = "radiance";
    static constexpr auto kNormal = "normal";
    static constexpr auto kOutput = "output";

    ChannelList _input = {
        {"radiance", "radiance", false, ResourceFormat::FLOAT4},
        {"normal", "normal", true, ResourceFormat::FLOAT4},
    };
    ChannelList _output = {
        {"output", "output", false, ResourceFormat::FLOAT4},
    };

public:
    [[nodiscard]] ChannelList inputs() const noexcept override { return _input; }
    [[nodiscard]] ChannelList outputs() const noexcept override { return _output; }
};

class TonemappingPass : public RenderPass {
private:
    static constexpr auto kInput = "input";
    static constexpr auto kOutput = "output";
    ChannelList _input = {
        {"input", "input", false, ResourceFormat::FLOAT4},
    };
    ChannelList _output = {
        {"output", "output", false, ResourceFormat::FLOAT4},
    };

public:
    [[nodiscard]] ChannelList inputs() const noexcept override { return _input; }
    [[nodiscard]] ChannelList outputs() const noexcept override { return _output; }
};

}// namespace vision

int main(int argc, char *argv[]) {

    RenderGraph graph;

    auto rt = new RTPass();
    auto r_accum = new AccumulatePass();
    auto denoise = new DenoisePass();
    auto tonemapping = new TonemappingPass();

    graph.add_pass(rt, "rt");
    graph.add_pass(r_accum, "accum");
    graph.add_pass(denoise, "denoise");
    graph.add_pass(tonemapping, "tonemapping");

//    graph.mark_output("tonemapping.output");
    graph.mark_output("accum.radiance");
    graph.add_edge("denoise.output", "tonemapping.input");
    graph.add_edge("accum.radiance", "denoise.radiance");
    //    graph.build_connection("accum.normal", "denoise.normal");
    graph.add_edge("rt.radiance", "accum.in_radiance");
    graph.add_edge("rt.normal", "denoise.normal");

    graph.setup();

    return 0;
}
