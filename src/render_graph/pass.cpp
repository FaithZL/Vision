//
// Created by Zero on 2023/6/24.
//

#include "pass.h"
#include "base/mgr/global.h"

namespace vision {

SP<RenderPass> RenderPass::create(const std::string &name, const vision::ParameterSet &ps) noexcept {
    PassDesc desc(name);
    desc.sub_type = name;
    desc.init(ps);
    return Node::create_shared<RenderPass>(desc);
}

}// namespace vision