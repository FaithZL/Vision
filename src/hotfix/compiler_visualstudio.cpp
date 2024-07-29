//
// Created by Zero on 2024/8/6.
//

#include "compiler.h"
#include <windows.h>

namespace vision::inline hotfix {

using namespace ocarina;

struct VSVersionInfo {
    fs::path path;
};

struct CmdProcessor {
public:
    PROCESS_INFORMATION cmd_process_info{};
    HANDLE cmd_process_read{};
    HANDLE cmd_process_write{};
    volatile bool is_complete{};
    bool m_bStoreCmdOutput{};
    std::string m_CmdOutput;

public:
    CmdProcessor() {
        ZeroMemory(&cmd_process_info, sizeof(cmd_process_info));
    }
    void write_input(ocarina::string& input) {

    }
    void cleanup() {

    }
    ~CmdProcessor() {
        cleanup();
    }
};

class CompilerVisualStudio : public Compiler {
private:
    CmdProcessor cmd_processor_;
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