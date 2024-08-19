//
// Created by Zero on 2024/7/26.
//

#pragma once

#include "core/stl.h"
#include "file_inspector.h"
#include "util/file_manager.h"
#include "core/dynamic_module.h"

namespace vision::inline hotfix {

enum OptimizationLevel {
    Default = 0,
    Debug = 1,
    Release = 2
};

struct BuildOptions {
    vector<fs::path> include_paths;
    vector<fs::path> library_paths;
    fs::path intermediate_path;
    OptimizationLevel optimization_level{Default};
    BuildOptions() {
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
        include_paths.push_back(src_dir);
        intermediate_path = FileInspector::intermediate_path();
        optimization_level = correct_optimization_level(optimization_level);
    }
};

class Compiler {
public:
    using Creator = Compiler *();
    using Deleter = void(Compiler *);
    using Handle = unique_ptr<Compiler, Deleter *>;

public:
    virtual void compile(const BuildOptions &options,
                         const FileInspector::Module &module) noexcept = 0;
    [[nodiscard]] virtual string get_object_file_extension() const noexcept = 0;
    [[nodiscard]] static fs::path cli_path() noexcept;
    [[nodiscard]] static Handle create(const string &name);
    [[nodiscard]] static Handle create();
};

}// namespace vision::inline hotfix