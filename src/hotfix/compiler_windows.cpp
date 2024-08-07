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
        vector<VSVersionInfo> vec;
        CompileOptions op;
        GetPathsOfVisualStudioInstalls(&vec);
    }
    void compile(const vision::CompileOptions &options) noexcept override {
    }
};

UP<Compiler> Compiler::create() noexcept {
    auto ret = make_unique<CompilerVisualStudio>();
    ret->init();
    return ret;
}

}// namespace vision::inline hotfix