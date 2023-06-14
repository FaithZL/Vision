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

Spectrum &Node::spectrum() noexcept {
    return pipeline()->spectrum();
}

const Spectrum &Node::spectrum() const noexcept {
    return pipeline()->spectrum();
}

Device &Node::device() noexcept {
    return pipeline()->device();
}


}// namespace vision