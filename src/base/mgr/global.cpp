//
// Created by Zero on 2023/6/14.
//

#include "global.h"
#include "pipeline.h"

namespace vision {

Global *Global::s_global = nullptr;

Global &Global::instance() {
    if (s_global == nullptr) {
        s_global = new Global();
    }
    return *s_global;
}

Global::~Global() {
    NodeMgr::destroy_instance();
    Context::destroy_instance();
}

void Global::set_pipeline(vision::Pipeline *pipeline) { _pipeline = pipeline; }

Pipeline *Global::pipeline() { return _pipeline; }

void Global::destroy_instance() {
    if (s_global) {
        delete s_global;
        s_global = nullptr;
    }
}
}// namespace vision