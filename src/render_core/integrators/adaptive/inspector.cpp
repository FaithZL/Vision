//
// Created by Zero on 2024/10/15.
//

#include "inspector.h"
#include "GUI/widgets.h"
#include "base/mgr/pipeline.h"

namespace vision {
ConvergenceInspector::ConvergenceInspector(const vision::ParameterSet &ps)
    : ConvergenceInspector(ps["threshold"].as_float(0.001f),
                           ps["start_index"].as_uint(128)) {}

void ConvergenceInspector::prepare() noexcept {
    variance_stats_.set_bindless_array(pipeline()->bindless_array());
    variance_stats_.super() = device().create_buffer<VarianceStats>(pipeline()->pixel_num());
    variance_stats_.register_self();
}

CommandList ConvergenceInspector::reset() noexcept {
    return pipeline()->reset_buffer<VarianceStats>(variance_stats_, {}, "reset_VarianceStats");
}

void ConvergenceInspector::add_sample(const Uint2 &pixel, const Float3 &value,
                                      const Uint &frame_index) noexcept {
    VarianceStatsVar vs = variance_stats_.read(dispatch_id());
    vs->add(luminance(value));
//    $condition_info("{} avg {}  var {}   rv  {}", vs.N, vs.avg, vs.var, vs->relative_variance());
    variance_stats_.write(dispatch_id(), vs);
}

Bool ConvergenceInspector::is_convergence(const Uint &frame_index) const noexcept {
    return false;
}

bool ConvergenceInspector::render_UI(ocarina::Widgets *widgets) noexcept {
    widgets->use_tree("adaptive sampling", [&] {
        render_sub_UI(widgets);
    });
    return true;
}

void ConvergenceInspector::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    changed_ |= widgets->drag_float("threshold", addressof(threshold_.hv()), 0.0001f, 0, 1, "%.5f");
    changed_ |=widgets->drag_uint("min frame index", addressof(start_index_.hv()), 1, 32, 1024);
}

}// namespace vision

VS_REGISTER_HOTFIX(vision, ConvergenceInspector)
VS_REGISTER_CURRENT_PATH(1, "vision-integrator-adaptive.dll")