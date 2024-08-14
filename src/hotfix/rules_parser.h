//
// Created by Zero on 2024/8/13.
//

#pragma once

#include "core/stl.h"

namespace vision::inline hotfix {

using namespace ocarina;

struct CompileOptions {
    vector<string> defines;
    vector<string> flags;
    vector<fs::path> includes;
    fs::path src;
    fs::path dst;
};

struct LinkOptions {
    vector<string> compile_flags;
    vector<fs::path> obj_files;
    vector<fs::path> link_libraries;
    fs::path target_file;
    fs::path target_implib;
};

class BuildRulesParser {
protected:
    vector<CompileOptions> compiles_;
    vector<LinkOptions> links_;

public:
    BuildRulesParser() = default;
};
}// namespace vision::inline hotfix