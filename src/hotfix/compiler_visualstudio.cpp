//
// Created by Zero on 2024/8/6.
//

#include "compiler.h"

#include "platform_win/cmd_process.h"
#include "platform_win/visual_studio_utils.h"

namespace vision::inline hotfix {

using namespace ocarina;


class CompilerVisualStudio : public Compiler {
private:
    CmdProcess cmd_process_;

public:
    void init() noexcept override {
    }
    void compile(const vision::CompileOptions &options) noexcept override {
    }
};

UP<Compiler> Compiler::create() noexcept {
    return make_unique<CompilerVisualStudio>();
}

}// namespace vision::inline hotfix