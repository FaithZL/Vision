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

struct CmdProcess {
public:
    PROCESS_INFORMATION cmd_process_info{};
    HANDLE cmd_process_read{};
    HANDLE cmd_process_write{};
    volatile bool is_complete{};
    bool store_cmd_output{};
    std::string cmd_output;

public:
    CmdProcess() {
        ZeroMemory(&cmd_process_info, sizeof(cmd_process_info));
    }

    void init_process() noexcept {
        STARTUPINFOW startup_info;
        ZeroMemory(&startup_info, sizeof(startup_info));
        startup_info.cb = sizeof(startup_info);

        // Set up the security attributes struct.
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = nullptr;
        sa.bInheritHandle = TRUE;

        startup_info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        startup_info.wShowWindow = SW_HIDE;
        HANDLE hOutputReadTmp = nullptr, hOutputWrite = nullptr, hErrorWrite = nullptr;

        auto error_func = [&] {
            if (hOutputReadTmp) {
                CloseHandle(hOutputReadTmp);
            }
            if (hOutputWrite) {
                CloseHandle(hOutputWrite);
            }
            if (hErrorWrite) {
                CloseHandle(hErrorWrite);
            }
        };

        if (!CreatePipe(&hOutputReadTmp, &hOutputWrite,
                        &sa, 20 * 1024)) {
            error_func();
            return;
        }
        startup_info.hStdOutput = hOutputWrite;

        if (!DuplicateHandle(GetCurrentProcess(), hOutputWrite,
                             GetCurrentProcess(), &hErrorWrite,
                             0, TRUE, DUPLICATE_SAME_ACCESS)) {
            error_func();
            return;
        }
        startup_info.hStdError = hErrorWrite;

        if (startup_info.hStdOutput) {
            if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp,
                                 GetCurrentProcess(),
                                 &cmd_process_read, // Address of new handle.
                                 0, FALSE, // Make it uninheritable.
                                 DUPLICATE_SAME_ACCESS))
            error_func;
            CloseHandle(hOutputReadTmp);
            hOutputReadTmp = nullptr;
            return;
        }
    }

    void write_input(ocarina::string &input) {
        DWORD nBytesWritten;
        DWORD length = (DWORD)input.length();
    }
    void cleanup() {
    }

    ~CmdProcess() {
        cleanup();
    }
};

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