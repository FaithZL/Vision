//
// Created by Zero on 2023/6/14.
//

#include "global.h"
#include "pipeline.h"
#include "hotfix/hotfix.h"

namespace vision {

Global *Global::s_global = nullptr;

Global &Global::instance() {
    if (s_global == nullptr) {
        s_global = new Global();
        HotfixSystem::instance().register_static_var("Global", *s_global);
    }
    return *s_global;
}

Global::~Global() {
    FileManager::destroy_instance();
    ImagePool::destroy_instance();
}

void Global::set_pipeline(vision::Pipeline *pipeline) { pipeline_ = pipeline; }

Pipeline *Global::pipeline() {
    OC_ASSERT(pipeline_);
    return pipeline_;
}

BindlessArray &Global::bindless_array() {
    return pipeline()->bindless_array();
}

void Global::set_scene_path(const fs::path &sp) noexcept {
    scene_path_ = sp;
}

fs::path Global::scene_path() const noexcept {
    return scene_path_;
}

fs::path Global::scene_cache_path() const noexcept {
    return scene_path_ / ".cache";
}

void Global::destroy_instance() {
    if (s_global) {
        delete s_global;
        s_global = nullptr;
    }
}

Pipeline *Context::pipeline() noexcept {
    return Global::instance().pipeline();
}

Device &Context::device() noexcept {
    return pipeline()->device();
}

Scene &Context::scene() noexcept {
    return pipeline()->scene();
}

FrameBuffer &Context::frame_buffer() noexcept {
    return *pipeline()->frame_buffer();
}

TSpectrum &Context::spectrum() noexcept {
    return scene().spectrum();
}

Stream &Context::stream() noexcept {
    return pipeline()->stream();
}

}// namespace vision