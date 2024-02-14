//
// Created by Zero on 2024/2/10.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/sensor/filter.h"
#include "base/denoiser.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"

namespace vision {
using namespace ocarina;

class SVGF;

class AtrousFilter : public Ctx {
private:
    SP<Filter> _filter;
    Buffer<float> _table;
    SVGF *_svgf{nullptr};
    using signature = void(Buffer<float4>, Buffer<float4>, Buffer<float4>,
                           Buffer<float4>, Buffer<float>, uint);
    Shader<signature> _shader;

public:
    explicit AtrousFilter(const Filter::Desc &desc, SVGF *svgf)
        : _filter(NodeMgr::instance().load<Filter>(desc)), _svgf(svgf) {}

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

    [[nodiscard]] CommandList dispatch(vision::DenoiseInput &input, uint step_width) noexcept {
        CommandList ret;
        ret << _shader(input.output->device_buffer(), input.color->device_buffer(), input.position->device_buffer(),
                       input.normal->device_buffer(), _table, step_width)
                   .dispatch(pipeline()->resolution());
        return ret;
    }
};

}// namespace vision