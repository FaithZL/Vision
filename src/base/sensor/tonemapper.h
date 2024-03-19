//
// Created by zhu on 2023/5/30.
//

#pragma once

#include "core/basic_types.h"
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
class ToneMapper : public Node, public Serializable<float> {
public:
    using Desc = ToneMapperDesc;

public:
    explicit ToneMapper(const ToneMapperDesc &desc)
        : Node(desc) {}
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        widgets->text("tone mapper: %s", impl_type().data());
        return true;
    }
    [[nodiscard]] virtual Float4 apply(const Float4 &input) const noexcept = 0;
};

}// namespace vision