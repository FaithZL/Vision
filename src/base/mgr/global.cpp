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
    ImagePool::destroy_instance();
}

void Global::set_pipeline(vision::Pipeline *pipeline) { _pipeline = pipeline; }

Pipeline *Global::pipeline() {
    OC_ASSERT(_pipeline);
    return _pipeline;
}

BindlessArray &Global::bindless_array() {
    return pipeline()->bindless_array();
}

void Global::set_scene_path(const fs::path &sp) noexcept {
    _scene_path = sp;
}

fs::path Global::scene_path() const noexcept {
    return _scene_path;
}

fs::path Global::scene_cache_path() const noexcept {
    return _scene_path / ".cache";
}

void Global::destroy_instance() {
    if (s_global) {
        delete s_global;
        s_global = nullptr;
    }
}

Pipeline *Ctx::pipeline() noexcept {
    return Global::instance().pipeline();
}

Device &Ctx::device() noexcept {
    return pipeline()->device();
}

Scene &Ctx::scene() noexcept {
    return pipeline()->scene();
}

Spectrum &Ctx::spectrum() noexcept {
    return *scene().spectrum();
}

Stream &Ctx::stream() noexcept {
    return pipeline()->stream();
}

}// namespace vision