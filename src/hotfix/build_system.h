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
    void build_targets(const vector<FileTool::Target> &targets) const noexcept;
    void build_target(const FileTool::Target &target) const noexcept;
    static void create_temp_path(const fs::path &path) noexcept;
    void compile(const FileTool::Target &target) const noexcept;
    void link(const FileTool::Target &target) const noexcept;
};

}// namespace vision::inline hotfix