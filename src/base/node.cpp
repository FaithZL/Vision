//
// Created by Zero on 16/01/2023.
//

#include "mgr/pipeline.h"
#include "node.h"
#include "mgr/global.h"

namespace vision {

Pipeline *Node::pipeline() noexcept {
    return Global::instance().pipeline();
}

Scene &Node::scene() noexcept {
    return pipeline()->scene();
}

fs::path Node::scene_path() noexcept {
    return Global::instance().scene_path();
}

Spectrum &Node::spectrum() noexcept {
    return *scene().spectrum();
}

Device &Node::device() noexcept {
    return pipeline()->device();
}

}// namespace vision