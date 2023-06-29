//
// Created by Zero on 2023/6/29.
//

#include "render_graph/pass.h"
#include "base/mgr/global.h"
#include "base/integrator.h"
#include "base/mgr/pipeline.h"

namespace vision {

class IntegratePass : public RenderPass {
private:
    Integrator *_integrator{};
    Shader<void(uint)> _shader;

public:
    explicit IntegratePass(const RenderPassDesc &desc)
        : RenderPass(desc) {
        IntegratorDesc integrator_desc;
        integrator_desc.init({});
        _integrator = NodeMgr::instance().load<Integrator>(integrator_desc);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IntegratePass)