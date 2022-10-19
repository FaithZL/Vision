//
// Created by Zero on 12/09/2022.
//


#include "base/integrator.h"

namespace vision {
class PathTracingIntegrator : public Integrator {
public:
    explicit PathTracingIntegrator(const IntegratorDesc *desc) : Integrator(desc) {}
};
}

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)