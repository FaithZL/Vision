//
// Created by Zero on 16/01/2023.
//

#include "mgr/render_pipeline.h"
#include "node.h"

namespace vision {

RenderPipeline *Node::render_pipeline() noexcept {
    return _scene->render_pipeline();
}

Device &Node::device() noexcept {
    return render_pipeline()->device();
}

}// namespace vision