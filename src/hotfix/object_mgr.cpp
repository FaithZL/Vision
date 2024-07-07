//
// Created by Zero on 2024/7/29.
//

#include "object_mgr.h"

namespace vision ::inline hotfix {

RuntimeObjectMgr *RuntimeObjectMgr::s_mgr = nullptr;

RuntimeObjectMgr &RuntimeObjectMgr::instance() noexcept {
    if (s_mgr == nullptr) {
        s_mgr = new RuntimeObjectMgr();
    }
    return *s_mgr;
}

void RuntimeObjectMgr::destroy_instance() noexcept {
    if (s_mgr == nullptr) {
        return;
    }
    delete s_mgr;
    s_mgr = nullptr;
}

RuntimeObjectMgr::~RuntimeObjectMgr() {}

}// namespace vision::inline hotfix