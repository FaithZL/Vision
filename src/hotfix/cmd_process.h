//
// Created by Zero on 2024/7/30.
//

#pragma once

#include <windows.h>
#include <thread>
#include "core/stl.h"

namespace vision::inline hotfix {
using namespace ocarina;
const std::string c_CompletionToken("_COMPLETION_TOKEN_");

struct CmdProcess;

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

    void init_process() noexcept;

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

void ReadAndHandleOutputThread(LPVOID arg) {
    auto pCmdProc = (CmdProcess *)arg;

    CHAR lpBuffer[1024];
    DWORD nBytesRead;
    bool bReadActive = true;
    while (bReadActive) {
        if (!ReadFile(pCmdProc->cmd_process_read, lpBuffer, sizeof(lpBuffer) - 1,
                      &nBytesRead, NULL) ||
            !nBytesRead) {
            bReadActive = false;
            if (GetLastError() != ERROR_BROKEN_PIPE)//broken pipe is OK
            {
                std::cout << "[RuntimeCompiler] Redirect of compile output failed on read\n"
                          << std::endl;
            }
        } else {
            // Add null termination
            lpBuffer[nBytesRead] = 0;

            //fist check for completion token...
            std::string buffer(lpBuffer);
            size_t found = buffer.find(c_CompletionToken);
            if (found != std::string::npos) {
                //we've found the completion token, which means we quit
                buffer = buffer.substr(0, found);
                if (!pCmdProc->store_cmd_output) {
                }
                pCmdProc->is_complete = true;
            }
            if (bReadActive || buffer.length())//don't output blank last line
            {
                if (pCmdProc->store_cmd_output) {
                    pCmdProc->cmd_output += buffer;
                } else {
                    //check if this is an error
                    size_t errorFound = buffer.find(" : error ");
                    size_t fatalErrorFound = buffer.find(" : fatal error ");
                    if ((errorFound != std::string::npos) || (fatalErrorFound != std::string::npos)) {

                    } else {
                    }
                }
            }
        }
    }
}

void CmdProcess::init_process() noexcept {
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
                             &cmd_process_read,// Address of new handle.
                             0, FALSE,         // Make it uninheritable.
                             DUPLICATE_SAME_ACCESS))
            error_func;
        CloseHandle(hOutputReadTmp);
        hOutputReadTmp = nullptr;
        return;
    }

    HANDLE hInputRead, hInputWriteTmp;
    // Create a pipe for the child process's STDIN.
    if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 4096)) {
        error_func();
    }
    startup_info.hStdInput = hInputRead;

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (startup_info.hStdOutput) {
        if (!DuplicateHandle(GetCurrentProcess(), hInputWriteTmp,
                             GetCurrentProcess(),
                             &cmd_process_write,// Address of new handle.
                             0, FALSE,          // Make it uninheritable.
                             DUPLICATE_SAME_ACCESS)) {
            error_func();
        }
    }

    const wchar_t *pCommandLine = L"cmd /q /K @PROMPT $";
    //CreateProcessW won't accept a const pointer, so copy to an array
    wchar_t pCmdLineNonConst[1024];
    wcscpy_s(pCmdLineNonConst, pCommandLine);
    CreateProcessW(
        NULL,            //__in_opt     LPCTSTR lpApplicationName,
        pCmdLineNonConst,//__inout_opt  LPTSTR lpCommandLine,
        NULL,            //__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
        NULL,            //__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
        TRUE,            //__in         BOOL bInheritHandles,
        0,               //__in         DWORD dwCreationFlags,
        NULL,            //__in_opt     LPVOID lpEnvironment,
        NULL,            //__in_opt     LPCTSTR lpCurrentDirectory,
        &startup_info,   //__in         LPSTARTUPINFO lpStartupInfo,
        &cmd_process_info//__out        LPPROCESS_INFORMATION lpProcessInformation
    );

    //launch threaded read.
    _beginthread(ReadAndHandleOutputThread, 0, this);//this will exit when process for compile is closed
}

}// namespace vision::inline hotfix
