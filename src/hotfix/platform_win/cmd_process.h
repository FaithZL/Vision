//
// Created by Zero on 2024/7/30.
//

#pragma once

#include <windows.h>
#include <thread>
#include "core/stl.h"
#include "core/logging.h"

namespace vision::inline hotfix {
using namespace ocarina;
const std::string c_CompletionToken("_COMPLETION_TOKEN_");

/// from https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/blob/master/Aurora/RuntimeCompiler/Compiler_PlatformWindows.cpp

struct CmdProcess {
    CmdProcess();
    ~CmdProcess();

    void InitialiseProcess();
    void WriteInput(std::string &input) const;
    void CleanupProcessAndPipes();

    void ReadAndHandleOutputThread() {
        CmdProcess *pCmdProc = this;

        CHAR lpBuffer[1024];
        DWORD nBytesRead;
        bool bReadActive = true;
        while (bReadActive) {
            if (!ReadFile(pCmdProc->m_CmdProcessOutputRead, lpBuffer, sizeof(lpBuffer) - 1,
                          &nBytesRead, nullptr) ||
                !nBytesRead) {
                bReadActive = false;
                if (GetLastError() != ERROR_BROKEN_PIPE) {
                    //broken pipe is OK
                    OC_WARNING("[RuntimeCompiler] Redirect of compile output failed on read\n");
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
                    if (!pCmdProc->m_bStoreCmdOutput) {
                        OC_INFO("[RuntimeCompiler] Complete\n");
                    }
                    pCmdProc->m_bIsComplete = true;
                }

                if (bReadActive || buffer.length()) {
                    //don't output blank last line
                    if (pCmdProc->m_bStoreCmdOutput) {
                        pCmdProc->m_CmdOutput += buffer;
                    } else {
                        //check if this is an error
                        size_t errorFound = buffer.find(" : error ");
                        size_t fatalErrorFound = buffer.find(" : fatal error ");
                        if ((errorFound != std::string::npos) || (fatalErrorFound != std::string::npos)) {
                            OC_WARNING_FORMAT("{}", buffer.c_str());
                        } else {
                            OC_INFO_FORMAT("{}", buffer.c_str());
                        }
                    }
                }
            }
        }
    }

    PROCESS_INFORMATION m_CmdProcessInfo{};
    HANDLE m_CmdProcessOutputRead;
    HANDLE m_CmdProcessInputWrite;
    std::atomic<bool> m_bIsComplete;
    bool m_bStoreCmdOutput;
    std::string m_CmdOutput;
    std::thread m_OutputThread;
};

CmdProcess::CmdProcess()
    : m_CmdProcessOutputRead(nullptr), m_CmdProcessInputWrite(nullptr),
      m_bIsComplete(false), m_bStoreCmdOutput(false) {
    ZeroMemory(&m_CmdProcessInfo, sizeof(m_CmdProcessInfo));
}

void CmdProcess::InitialiseProcess() {
    //init compile process
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    // Set up the security attributes struct.
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;

    // Create the child output pipe.
    //redirection of output
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    HANDLE hOutputReadTmp = nullptr, hOutputWrite = nullptr, hErrorWrite = nullptr;
    auto exit_func = [&] {
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
    if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 20 * 1024)) {
        OC_WARNING("[RuntimeCompiler] Failed to create output redirection pipe\n");
        exit_func();
        return;
    }
    si.hStdOutput = hOutputWrite;

    // Create a duplicate of the output write handle for the std error
    // write handle. This is necessary in case the child application
    // closes one of its std output handles.
    if (!DuplicateHandle(GetCurrentProcess(), hOutputWrite,
                         GetCurrentProcess(), &hErrorWrite, 0,
                         TRUE, DUPLICATE_SAME_ACCESS)) {
        OC_WARNING("[RuntimeCompiler] Failed to duplicate error output redirection pipe\n");
        exit_func();
        return;
    }
    si.hStdError = hErrorWrite;

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (si.hStdOutput) {
        if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp,
                             GetCurrentProcess(),
                             &m_CmdProcessOutputRead,// Address of new handle.
                             0, FALSE,               // Make it uninheritable.
                             DUPLICATE_SAME_ACCESS)) {
            OC_WARNING("[RuntimeCompiler] Failed to duplicate output read pipe\n");
            exit_func();
            return;
        }
        CloseHandle(hOutputReadTmp);
        hOutputReadTmp = nullptr;
    }

    HANDLE hInputRead, hInputWriteTmp;
    // Create a pipe for the child process's STDIN.
    if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 4096)) {
        OC_WARNING("[RuntimeCompiler] Failed to create input pipes\n");
        exit_func();
        return;
    }
    si.hStdInput = hInputRead;

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (si.hStdOutput && !DuplicateHandle(GetCurrentProcess(), hInputWriteTmp,
                                          GetCurrentProcess(),
                                          &m_CmdProcessInputWrite,// Address of new handle.
                                          0, FALSE,               // Make it uninheritable.
                                          DUPLICATE_SAME_ACCESS)) {
        OC_WARNING("[RuntimeCompiler] Failed to duplicate input write pipe\n");
        exit_func();
        return;
    }

    const wchar_t *pCommandLine = L"cmd /q /K @PROMPT $";
    //CreateProcessW won't accept a const pointer, so copy to an array
    wchar_t pCmdLineNonConst[1024];
    wcscpy_s(pCmdLineNonConst, pCommandLine);
    CreateProcessW(
        nullptr,         //__in_opt     LPCTSTR lpApplicationName,
        pCmdLineNonConst,//__inout_opt  LPTSTR lpCommandLine,
        nullptr,         //__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
        nullptr,         //__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
        TRUE,            //__in         BOOL bInheritHandles,
        0,               //__in         DWORD dwCreationFlags,
        nullptr,         //__in_opt     LPVOID lpEnvironment,
        nullptr,         //__in_opt     LPCTSTR lpCurrentDirectory,
        &si,             //__in         LPSTARTUPINFO lpStartupInfo,
        &m_CmdProcessInfo//__out        LPPROCESS_INFORMATION lpProcessInformation
    );

    //launch threaded read.
    m_OutputThread = std::thread(&CmdProcess::ReadAndHandleOutputThread, this);
    m_OutputThread.detach();

    exit_func();
}

void CmdProcess::WriteInput(std::string &input) const {
    DWORD nBytesWritten;
    DWORD length = (DWORD)input.length();
    WriteFile(m_CmdProcessInputWrite, input.c_str(), length, &nBytesWritten, nullptr);
}

void CmdProcess::CleanupProcessAndPipes() {
    if (m_CmdProcessInfo.hProcess) {
        TerminateProcess(m_CmdProcessInfo.hProcess, 0);
        TerminateThread(m_CmdProcessInfo.hThread, 0);
        CloseHandle(m_CmdProcessInfo.hThread);
        ZeroMemory(&m_CmdProcessInfo, sizeof(m_CmdProcessInfo));
        CloseHandle(m_CmdProcessInputWrite);
        m_CmdProcessInputWrite = 0;
        CloseHandle(m_CmdProcessOutputRead);
        m_CmdProcessOutputRead = 0;
    }
}

CmdProcess::~CmdProcess() {
    CleanupProcessAndPipes();
}

}// namespace vision::inline hotfix
