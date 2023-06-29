//
// Created by Zero on 2023/6/24.
//

#include "pass.h"
#include "base/mgr/global.h"

namespace vision {

RenderPass *RenderPass::create(const std::string &name, const vision::ParameterSet &ps) noexcept {
    RenderPassDesc desc(name);
    desc.sub_type = name;
    desc.init(ps);
    return NodeMgr::instance().load<RenderPass>(desc);
}

}// namespace vision