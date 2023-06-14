//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "node_mgr.h"
#include "rhi/context.h"

namespace vision {
class Pipeline;
class Global {
private:
    Global() = default;
    Global(const Global &) = delete;
    Global(Global &&) = delete;
    Global operator=(const Global &) = delete;
    Global operator=(Global &&) = delete;
    static Global *s_global;
    ~Global();

private:
    Pipeline *_pipeline{nullptr};

public:
    [[nodiscard]] static Global &instance();
    static void destroy_instance();
    void set_pipeline(Pipeline *pipeline);
    [[nodiscard]] Pipeline *pipeline();
    [[nodiscard]] static decltype(auto) node_mgr() {
        return NodeMgr::instance();
    }
    [[nodiscard]] static decltype(auto) context() {
        return Context::instance();
    }
};

}// namespace vision