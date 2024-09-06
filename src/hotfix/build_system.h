//
// Created by Zero on 2024/8/2.
//

#include "compiler.h"
#include "build_rules.h"
#include "cmd_process.h"

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
    void build_targets(const vector<Target> &targets,
                       const std::function<void(bool, Target)> &callback = nullptr) const noexcept;
    void build_target(const Target &target,
                      const CmdProcess::callback_t& callback = nullptr) const noexcept;
    static void create_temp_path(const fs::path &path) noexcept;
    [[nodiscard]] bool is_working() const noexcept { return compiler_->is_working(); }
    void compile(const Target &target) const noexcept;
    void link(const Target &target, const CmdProcess::callback_t &callback = nullptr) const noexcept;
};

}// namespace vision::inline hotfix