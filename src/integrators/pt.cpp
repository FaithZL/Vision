//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "core/render_pipeline.h"

namespace vision {
class PathTracingIntegrator : public Integrator {
public:
    explicit PathTracingIntegrator(const IntegratorDesc &desc) : Integrator(desc) {}

    void render(RenderPipeline *rp) const noexcept override {

    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)