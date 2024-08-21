//
// Created by Zero on 2024/8/2.
//

#include "compiler.h"
#include "build_rules.h"

namespace vision::inline hotfix {
using namespace ocarina;

class BuildSystem {
private:
    BuildRules::Handle build_rules_{BuildRules::Handle(nullptr, nullptr)};
    mutable Compiler::Handle compiler_{Compiler::Handle{nullptr, nullptr}};

public:
    BuildSystem();
    void init();
    void clear() noexcept;
    [[nodiscard]] static fs::path directory() noexcept;
    void build_modules(const vector<FileInspector::Target> &targets) const noexcept;
    void build_module(const FileInspector::Target &module) const noexcept;
    void compile(const FileInspector::Target &target) const noexcept;
    void link(const FileInspector::Target &target) const noexcept;
};

}// namespace vision::inline hotfix