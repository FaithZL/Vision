//
// Created by Zero on 2024/2/10.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/sensor/filter.h"
#include "base/denoiser.h"
#include "base/mgr/global.h"

namespace vision {
using namespace ocarina;

class AtrousFilter : public Ctx {
private:
    SP<Filter> _filter;
    Buffer<float> _table;
    using signature = void(Buffer<float4>, Buffer<float4>, Buffer<float4>,
                           Buffer<float4>, Buffer<float>, uint);
    Shader<signature> _shader;

public:
    explicit AtrousFilter(const Filter::Desc &desc)
        : _filter(NodeMgr::instance().load<Filter>(desc)) {}

    void prepare() noexcept {
        vector<float> tab = _filter->discretize(5);
        _table = device().create_buffer<float>(tab.size());
        _table.upload_immediately(tab.data());
    }

    void compile() noexcept {
        Kernel kernel = [&](BufferVar<float4> output, BufferVar<float4> rt, BufferVar<float4> pos,
                            BufferVar<float4> normal, BufferVar<float> convolution, Uint step_width) {

        };
        _shader = device().compile(kernel, "SVGF-atrous");
    }

    void apply(vision::DenoiseInput &input) noexcept {
    }
};

}// namespace vision