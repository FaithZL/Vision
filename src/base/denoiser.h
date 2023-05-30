//
// Created by Zero on 2023/5/30.
//

#pragma once

#include "dsl/common.h"
#include "node.h"

namespace vision {

class Denoiser : public Node {
public:
    using Desc = DenoiserDesc;
};

}