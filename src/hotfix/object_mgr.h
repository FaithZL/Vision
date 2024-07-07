//
// Created by Zero on 2024/7/29.
//

#pragma once

#include "core/stl.h"

namespace vision::inline hotfix {

using namespace ocarina;

class RuntimeObject {

};

class RuntimeObjectMgr {
private:
    RuntimeObjectMgr() = default;
    ~RuntimeObjectMgr();
    static RuntimeObjectMgr *s_mgr;

private:
    vector<RuntimeObject> objects_;

public:
    RuntimeObjectMgr(const RuntimeObjectMgr &) = delete;
    RuntimeObjectMgr(RuntimeObjectMgr &&) = delete;
    RuntimeObjectMgr operator=(const RuntimeObjectMgr &) = delete;
    RuntimeObjectMgr operator=(RuntimeObjectMgr &&) = delete;
    static RuntimeObjectMgr &instance() noexcept;
    static void destroy_instance() noexcept;
};

}// namespace vision::inline hotfix
