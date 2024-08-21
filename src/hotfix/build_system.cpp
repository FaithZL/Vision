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
    fs::path fn("bin");
    fn = fn / target.name;
    const LinkOptions &options = build_rules_->link_options(fn.string());
    compiler_->link(options, target);
}

void BuildSystem::create_immediate_path(const fs::path &path) noexcept {
    if (fs::exists(path)) {
        return;
    }
    fs::create_directory(path);
}

void BuildSystem::build_target(const FileInspector::Target &target) const noexcept {
    fs::path im_path = FileInspector::intermediate_path() / fs::path(target.name).stem();
    create_immediate_path(im_path);
    compiler_->setup_environment();
    compile(target);
    link(target);
}

void BuildSystem::build_targets(const vector<FileInspector::Target> &targets) const noexcept {
    std::for_each(targets.begin(), targets.end(), [&](const auto &target) {
        OC_INFO_FORMAT("target {} has been modified", target.name);
        for (const fs::path &fn : target.modified_files) {
            OC_INFO_FORMAT("file: {}", fn.string());
        }
        build_target(target);
    });
}

}// namespace vision::inline hotfix