//
// Created by zhu on 2023/5/30.
//

#pragma once

#include "core/basic_types.h"
#include "rhi/common.h"
#include "dsl/common.h"
#include "node.h"

namespace vision {

//"tone_mapping": {
//    "type": "gamma",
//    "param": {
//
//    }
//}
class ToneMapping : public Node {
public:
    using Desc = ToneMappingDesc;

public:
    explicit ToneMapping(const ToneMappingDesc &desc)
        : Node(desc) {}

    [[nodiscard]] virtual Float4 apply(const Float4 &input) const noexcept = 0;
};

}// namespace vision