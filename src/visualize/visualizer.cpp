//
// Created by Zero on 2024/9/21.
//

#include "visualizer.h"

namespace vision {

void Visualizer::init() noexcept {

}

bool Visualizer::render_UI(ocarina::Widgets *widgets) noexcept {
    return true;
}

void Visualizer::render_sub_UI(ocarina::Widgets *widgets) noexcept {
}

void Visualizer::draw(const ocarina::float4 *data, ocarina::uint2 res) const noexcept {
}

void Visualizer::clear() noexcept {
}

}// namespace vision

VS_REGISTER_HOTFIX(vision, Visualizer)
VS_REGISTER_CURRENT_PATH(0, "vision-visualize.dll")