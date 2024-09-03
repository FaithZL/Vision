//
// Created by Zero on 2024/7/30.
//

#pragma once

#include <windows.h>
#include <thread>
#include <utility>
#include "core/stl.h"
#include "core/logging.h"
#include "core/thread_safety.h"

namespace vision::inline hotfix {
using namespace ocarina;
static constexpr std::string_view c_CompletionToken("_COMPLETION_TOKEN_");

/// from https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/blob/master/Aurora/RuntimeCompiler/Compiler_PlatformWindows.cpp

class CmdProcess : public thread_safety<> {
public:
    using callback_t = std::function<void(const string &, bool)>;
    struct CmdData {
        string cmd;
        std::atomic<bool> success{true};
        callback_t callback;
        CmdData(string cmd, callback_t cb) : cmd(ocarina::move(cmd)), callback(std::move(cb)) {}
        void enqueue_callback() noexcept;
    };

private:
    PROCESS_INFORMATION process_info_{};
    HANDLE output_read_{};
    HANDLE input_write_{};
    bool store_cmd_output_{};
    std::string cmd_output_;
    std::thread output_thread_;
    mutable std::queue<CmdData> cmd_queue_;

public:
    CmdProcess();
    ~CmdProcess();

    void initialise();
    void change_directory(const fs::path &dir) const noexcept;
    void write_input(std::string input, const callback_t &callback = nullptr) const;
    void cleanup_process();

    [[nodiscard]] static string add_complete_flag(const string &cmd) noexcept {
        return cmd + ocarina::format("\n echo {} \n", c_CompletionToken);
    }

private:
    void read_output_thread();
    void on_finish_cmd() const;
};

}// namespace vision::inline hotfix
