//
// Created by Zero on 2024/8/2.
//

#include "build_tool.h"
#include "core/logging.h"

namespace vision::inline hotfix {

void BuildTool::clear() noexcept {
}

void BuildTool::build_modules(const vector<FileInspector::Module> &modules) const noexcept {
    std::for_each(modules.begin(), modules.end(), [&](const auto &module) {
        OC_INFO_FORMAT("module {} has been modified", module.name);
        for(const fs::path &fn: module.modified_files) {
            OC_INFO_FORMAT("file: {}", fn.string());
        }
    });
}

}// namespace vision::inline hotfix