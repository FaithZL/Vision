//
// Created by Zero on 2024/8/2.
//

#include "compiler.h"
#include "build_rules.h"

namespace vision::inline hotfix {
using namespace ocarina;

class BuildSystem {
private:
    BuildRules::Handle build_rules_{BuildRules::create("ninja")};
    mutable Compiler::Handle compiler_{Compiler::Handle{nullptr, nullptr}};

public:
    BuildSystem();
    void clear() noexcept;
    void build_modules(const vector<FileInspector::Module> &modules) const noexcept;
    void build_module(const FileInspector::Module &module) const noexcept;
};

}// namespace vision::inline hotfix