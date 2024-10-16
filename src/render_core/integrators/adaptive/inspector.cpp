//
// Created by Zero on 2024/10/15.
//

#include "inspector.h"

namespace vision {
ConvergenceInspector::ConvergenceInspector(const vision::ParameterSet &ps)
    : ConvergenceInspector(ps["threshold"].as_float(0.01f),
                           ps["start_index"].as_uint(128)) {}
}

VS_REGISTER_HOTFIX(vision, ConvergenceInspector)
VS_REGISTER_CURRENT_PATH(1, "vision-integrator-adaptive.dll")