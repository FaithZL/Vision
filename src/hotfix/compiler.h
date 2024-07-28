//
// Created by Zero on 2024/7/26.
//

#pragma once

#include "core/stl.h"
#include "file_inspector.h"

namespace vision::inline hotfix {

enum OptimizationLevel {
    Default = 0,
    Debug = 1,
    Release = 2
};

struct CompileOptions {
    vector<fs::path> include_paths;
    vector<fs::path> library_paths;
    fs::path intermediate_path;
    CompileOptions() {
        init();
    }
    void init() {
        fs::path src_dir = FileInspector::project_path() / "src";
        include_paths.push_back(src_dir / "ocarina" / "src");
        include_paths.push_back(src_dir);
    }
};

class Compiler {
public:
    virtual void init() noexcept = 0;
    virtual void compile(const CompileOptions &options) noexcept = 0;
};

}// namespace vision::inline hotfix