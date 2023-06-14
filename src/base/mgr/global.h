//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "node_mgr.h"
#include "rhi/context.h"

namespace vision {

class Global {
private:
    Global() = default;
    Global(const Global &) = delete;
    Global(Global &&) = delete;
    Global operator=(const Global &) = delete;
    Global operator=(Global &&) = delete;
    static Global *s_global;

public:
    [[nodiscard]] static Global &instance();
    static void destroy_instance();
};

}// namespace vision