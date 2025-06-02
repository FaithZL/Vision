//
// Created by Zero on 16/01/2023.
//

#include "mgr/pipeline.h"
#include "node.h"
#include "mgr/global.h"
#include "GUI/widgets.h"

namespace vision {

Pipeline *Node::pipeline() noexcept {
    return Global::instance().pipeline();
}

Scene &Node::scene() noexcept {
    return pipeline()->scene();
}

bool Node::render_UI(ocarina::Widgets *widgets) noexcept {
    widgets->text("%s, %s-%s does not implement render_UI", name_.c_str(),
                  category().data(), impl_type().data());
    return true;
}

fs::path Node::scene_path() noexcept {
    return Global::instance().scene_path();
}

TSpectrum &Node::spectrum() noexcept {
    return scene().spectrum();
}

FrameBuffer &Node::frame_buffer() noexcept {
    return *pipeline()->frame_buffer();
}

Device &Node::device() noexcept {
    return pipeline()->device();
}

}// namespace vision