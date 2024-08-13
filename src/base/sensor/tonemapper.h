//
// Created by zhu on 2023/5/30.
//

#pragma once

#include "math/basic_types.h"
#include "rhi/common.h"
#include "dsl/dsl.h"
#include "GUI/widgets.h"
#include "base/node.h"

namespace vision {

//"tone_mapping": {
//    "type": "linear",
//    "param": {
//
//    }
//}
class ToneMapper : public Node, public Encodable<float> {
public:
    using Desc = ToneMapperDesc;

public:
    explicit ToneMapper(const ToneMapperDesc &desc)
        : Node(desc) {}
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        return widgets->use_folding_header(ocarina::format("{} tone mapper", impl_type().data()),[&]{
            render_sub_UI(widgets);
        });
    }
    [[nodiscard]] virtual Float4 apply(const Float4 &input) const noexcept = 0;
};

using TToneMapper = TObject<ToneMapper>;

}// namespace vision