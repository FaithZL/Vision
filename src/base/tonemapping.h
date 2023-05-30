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
//    "type": "aces",
//    "param": {
//        "resolution": [
//            1280,
//            720
//            ],
//         "fb_state": 0
//    }
//}
class ToneMapping : public Node {
public:
    using Desc = ToneMappingDesc;
};

}// namespace vision