//
// Created by Zero on 2024/8/2.
//

#include "build_system.h"
#include "core/logging.h"

namespace vision::inline hotfix {

BuildSystem::BuildSystem() = default;

void BuildSystem::clear() noexcept {
}

void BuildSystem::build_module(const FileInspector::Module &module) const noexcept {
    BuildOptions compile_options;

    compiler_->compile(compile_options, module);
}

void BuildSystem::build_modules(const vector<FileInspector::Module> &modules) const noexcept {
    if (compiler_ == nullptr) {
        compiler_ = Compiler::create();
    }
    std::for_each(modules.begin(), modules.end(), [&](const auto &module) {
        OC_INFO_FORMAT("module {} has been modified", module.name);
        for (const fs::path &fn : module.modified_files) {
            OC_INFO_FORMAT("file: {}", fn.string());
        }
        build_module(module);
    });
}

}// namespace vision::inline hotfix