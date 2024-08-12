//
// Created by Zero on 2024/7/26.
//

#pragma once

#include "core/stl.h"
#include "file_inspector.h"
#include "util/file_manager.h"

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
    OptimizationLevel optimization_level{Default};
    CompileOptions() {
        init();
    }

    static OptimizationLevel correct_optimization_level(OptimizationLevel level) noexcept {
        switch (level) {
            case Default:
#ifndef NDEBUG
                return OptimizationLevel::Debug;
#else
                return OptimizationLevel::Release;
#endif
            default:
                return level;
        }
    }

    void init() {
        fs::path src_dir = FileInspector::project_src_path();

        include_paths = ocarina::include_paths();

//        include_paths.push_back(src_dir / "ocarina" / "src");
//        include_paths.push_back(src_dir / "ocarina" / "src" / "ext" / "xxHash");
//        include_paths.push_back(src_dir / "ocarina" / "src" / "ext" / "EASTL" / "include");
//        include_paths.push_back(src_dir / "ocarina" / "src" / "ext" / "EASTL" / "packages" /"EABase" / "include" / "Common");
//        include_paths.push_back(src_dir / "ocarina" / "src" / "ext" / "fmt" / "include");
        include_paths.push_back(src_dir);
//        paths.push_back(src_dir);
        intermediate_path = FileInspector::intermediate_path();
        optimization_level = correct_optimization_level(optimization_level);
    }
};

class Compiler {
public:
    virtual void init() noexcept = 0;
    virtual void compile(const CompileOptions &options,
                         const FileInspector::Module &module) noexcept = 0;
    [[nodiscard]] virtual string get_object_file_extension() const noexcept = 0;
    [[nodiscard]] static UP<Compiler> create() noexcept;
};

}// namespace vision::inline hotfix