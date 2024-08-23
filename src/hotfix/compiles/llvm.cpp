//
// Created by Zero on 2024/8/19.
//

#include "hotfix/compiler.h"

namespace vision::inline hotfix {

class LLVMCompiler : public Compiler {
public:
    LLVMCompiler();
    void compile(const vision::CompileOptions &options) noexcept override;
    void link(const vision::LinkOptions &options, const FileInspector::Target &target,
              const string &extension_objs) noexcept override;
};

LLVMCompiler::LLVMCompiler() {
}

void LLVMCompiler::compile(const CompileOptions &options) noexcept {
}

void LLVMCompiler::link(const LinkOptions &options, const FileInspector::Target &target,
                        const string &extension_objs) noexcept {
}

}// namespace vision::inline hotfix