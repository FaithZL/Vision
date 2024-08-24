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

void BuildSystem::compile(const FileTool::Target &target) const noexcept {
    for (const fs::path &item : target.modified_files) {
        const CompileOptions &options = build_rules_->compile_options(item.string());
        compiler_->compile(options);
    }
}

void BuildSystem::link(const FileTool::Target &target, const CmdProcess::callback_t &callback) const noexcept {
    fs::path fn("bin");
    fn = fn / target.name;
    const LinkOptions &options = build_rules_->link_options(fn.string());
    vector<string> extension_objs = build_rules_->obj_paths(ModuleInterface::src_path());
    compiler_->link(options, target, FileTool::files_string(extension_objs), callback);
}

void BuildSystem::create_temp_path(const fs::path &path) noexcept {
    if (fs::exists(path)) {
        return;
    }
    fs::create_directory(path);
}

void BuildSystem::build_target(const FileTool::Target &target,
                               const CmdProcess::callback_t &callback) const noexcept {
    create_temp_path(target.temp_directory());
    compiler_->setup_environment();
    compile(target);
    link(target,callback);
}

void BuildSystem::build_targets(const vector<FileTool::Target> &targets,
                                const CmdProcess::callback_t &callback) const noexcept {
    for (int i = 0; i < targets.size(); ++i) {
        const auto &target = targets.at(i);
        OC_INFO_FORMAT("target {} has been modified", target.name);
        for (const fs::path &fn : target.modified_files) {
            OC_INFO_FORMAT("file: {}", fn.string());
        }
        if (i == targets.size() - 1) {
            build_target(target, callback);
        } else {
            build_target(target);
        }
    }
}

}// namespace vision::inline hotfix