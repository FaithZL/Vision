//
// Created by Zero on 2024/8/2.
//

#include "build_system.h"
#include "core/logging.h"

namespace vision::inline hotfix {

BuildSystem::BuildSystem() = default;

void BuildSystem::init() {
    build_rules_ = BuildRules::create();
    compiler_ = Compiler::create();
}

void BuildSystem::clear() noexcept {
}

fs::path BuildSystem::directory() noexcept {
    return fs::current_path().parent_path();
}

void BuildSystem::compile(const FileInspector::Target &target) const noexcept {
    for (const fs::path &item : target.modified_files) {
        const CompileOptions &options = build_rules_->compile_options(item.string());
        compiler_->compile(options);
    }
}

void BuildSystem::link(const FileInspector::Target &target) const noexcept {

}

void BuildSystem::build_module(const FileInspector::Target &target) const noexcept {
    compiler_->setup_environment();
    compile(target);
    link(target);
}

void BuildSystem::build_modules(const vector<FileInspector::Target> &targets) const noexcept {
    std::for_each(targets.begin(), targets.end(), [&](const auto &target) {
        OC_INFO_FORMAT("target {} has been modified", target.name);
        for (const fs::path &fn : target.modified_files) {
            OC_INFO_FORMAT("file: {}", fn.string());
        }
        build_module(target);
    });
}

}// namespace vision::inline hotfix