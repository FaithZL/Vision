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
public:
    PROCESS_INFORMATION m_CmdProcessInfo{};
    HANDLE m_CmdProcessOutputRead{};
    HANDLE m_CmdProcessInputWrite{};
    std::atomic<bool> m_bIsComplete;
    bool m_bStoreCmdOutput{};
    std::string m_CmdOutput;
    std::thread m_OutputThread;
    CmdProcess();
    ~CmdProcess();

    void InitialiseProcess();
    void WriteInput(std::string &input) const;
    void CleanupProcessAndPipes();

private:
    void ReadAndHandleOutputThread();
};

CmdProcess::CmdProcess() {
    ZeroMemory(&m_CmdProcessInfo, sizeof(m_CmdProcessInfo));
}

void CmdProcess::ReadAndHandleOutputThread() {
    CHAR lpBuffer[1024];
    DWORD nBytesRead;
    while (true) {
        if (!ReadFile(m_CmdProcessOutputRead, lpBuffer, sizeof(lpBuffer) - 1,
                      &nBytesRead, nullptr) ||
            !nBytesRead) {
            if (GetLastError() != ERROR_BROKEN_PIPE) {
                std::cerr << "[RuntimeCompiler] Redirect of compile output failed on read" << std::endl;
            }
            break;
        }

        lpBuffer[nBytesRead] = 0;// Add null termination
        std::string buffer(lpBuffer);
        size_t found = buffer.find(c_CompletionToken);
        if (found != std::string::npos) {
            buffer = buffer.substr(0, found);
            if (!m_bStoreCmdOutput) {
                std::cout << "[RuntimeCompiler] Complete" << std::endl;
            }
            m_bIsComplete = true;
        }

        if (buffer.empty()) {
            continue;
        }
        if (m_bStoreCmdOutput) {
            m_CmdOutput += buffer;
            continue;
        }
        if (buffer.find(" : error ") != std::string::npos ||
            buffer.find(" : fatal error ") != std::string::npos) {
            std::cerr << "Warning: " << buffer << std::endl;
        } else {
            std::cout << buffer << std::endl;
        }
    }
}

void CmdProcess::InitialiseProcess() {
    //init compile process
    STARTUPINFOW startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    // Set up the security attributes struct.
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;

    // Create the child output pipe.
    //redirection of output
    startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE;
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
    startupInfo.hStdOutput = hOutputWrite;

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
    startupInfo.hStdError = hErrorWrite;

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (startupInfo.hStdOutput) {
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
    startupInfo.hStdInput = hInputRead;

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (startupInfo.hStdOutput && !DuplicateHandle(GetCurrentProcess(), hInputWriteTmp,
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
        &startupInfo,             //__in         LPSTARTUPINFO lpStartupInfo,
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
        m_CmdProcessInputWrite = nullptr;
        CloseHandle(m_CmdProcessOutputRead);
        m_CmdProcessOutputRead = nullptr;
    }
}

CmdProcess::~CmdProcess() {
    CleanupProcessAndPipes();
}

}// namespace vision::inline hotfix
