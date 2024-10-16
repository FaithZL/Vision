//
// Created by Zero on 2024/10/15.
//

#include "inspector.h"
#include "GUI/widgets.h"

namespace vision {
ConvergenceInspector::ConvergenceInspector(const vision::ParameterSet &ps)
    : ConvergenceInspector(ps["threshold"].as_float(0.01f),
                           ps["start_index"].as_uint(128)) {}

void ConvergenceInspector::add_sample(const Uint2 &pixel, const Float3 &value,
                                      const Uint &frame_index) noexcept {

}

Bool ConvergenceInspector::is_convergence() const noexcept {
    return false;
}

bool ConvergenceInspector::render_UI(ocarina::Widgets *widgets) noexcept {
    widgets->use_tree("adaptive sampling", [&] {
        render_sub_UI(widgets);
    });
    return true;
}

void ConvergenceInspector::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    widgets->drag_float("threshold", addressof(threshold_.hv()), 0.01f, 0, 10);
    widgets->drag_uint("min frame index", addressof(start_index_.hv()), 1, 32, 1024);
}

}// namespace vision

VS_REGISTER_HOTFIX(vision, ConvergenceInspector)
VS_REGISTER_CURRENT_PATH(1, "vision-integrator-adaptive.dll")