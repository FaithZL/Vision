//
// Created by Zero on 2024/7/30.
//

#pragma once

#include <Windows.h>
#include <thread>
#include "core/stl.h"
#include "core/logging.h"

namespace vision::inline hotfix {
using namespace ocarina;
const std::string c_CompletionToken("_COMPLETION_TOKEN_");

struct CmdProcess {
    CmdProcess();
    ~CmdProcess();

    void InitialiseProcess();
    void WriteInput(std::string &input);
    void CleanupProcessAndPipes();

    PROCESS_INFORMATION m_CmdProcessInfo;
    HANDLE m_CmdProcessOutputRead;
    HANDLE m_CmdProcessInputWrite;
    volatile bool m_bIsComplete;
    bool m_bStoreCmdOutput;
    std::string m_CmdOutput;
    fs::path m_PathTempCLCommandFile;
};

CmdProcess::CmdProcess()
    : m_CmdProcessOutputRead(NULL), m_CmdProcessInputWrite(NULL),
      m_bIsComplete(false), m_bStoreCmdOutput(false) {
    ZeroMemory(&m_CmdProcessInfo, sizeof(m_CmdProcessInfo));
}


void ReadAndHandleOutputThread( LPVOID arg )
{
    CmdProcess* pCmdProc = (CmdProcess*)arg;

    CHAR lpBuffer[1024];
    DWORD nBytesRead;
    bool bReadActive = true;
    while( bReadActive )
    {
        if( !ReadFile( pCmdProc->m_CmdProcessOutputRead,lpBuffer,sizeof(lpBuffer)-1,
                      &nBytesRead,NULL) || !nBytesRead)
        {
            bReadActive = false;
            if( GetLastError() != ERROR_BROKEN_PIPE)	//broken pipe is OK
            {
                OC_WARNING( "[RuntimeCompiler] Redirect of compile output failed on read\n" );
            }
        }
        else
        {
            // Add null termination
            lpBuffer[nBytesRead]=0;
            //fist check for completion token...
            std::string buffer( lpBuffer );
            size_t found = buffer.find( c_CompletionToken );
            if( found != std::string::npos )
            {
                //we've found the completion token, which means we quit
                buffer = buffer.substr( 0, found );
                if( !pCmdProc->m_bStoreCmdOutput ) {
                    OC_INFO("[RuntimeCompiler] Complete\n");
                }
                pCmdProc->m_bIsComplete = true;
            }

            if( bReadActive || buffer.length() ) //don't output blank last line
            {
                if( pCmdProc->m_bStoreCmdOutput )
                {
                    pCmdProc->m_CmdOutput += buffer;
                }
                else
                {
                    //check if this is an error
                    size_t errorFound = buffer.find( " : error " );
                    size_t fatalErrorFound = buffer.find( " : fatal error " );
                    if( ( errorFound != std::string::npos ) || ( fatalErrorFound != std::string::npos ) )
                    {
                        OC_WARNING_FORMAT( "{}", buffer.c_str() );
                    }
                    else
                    {
                        OC_INFO_FORMAT( "{}", buffer.c_str() );
                    }
                }
            }

        }
    }

}

void CmdProcess::InitialiseProcess()
{
    //init compile process
    STARTUPINFOW				si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    // Set up the security attributes struct.
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;


    // Create the child output pipe.
    //redirection of output
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    HANDLE hOutputReadTmp = NULL, hOutputWrite = NULL, hErrorWrite = NULL;
    if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 20 * 1024))
    {
        OC_WARNING("[RuntimeCompiler] Failed to create output redirection pipe\n");
//        goto ERROR_EXIT;
    }
    si.hStdOutput = hOutputWrite;

    // Create a duplicate of the output write handle for the std error
    // write handle. This is necessary in case the child application
    // closes one of its std output handles.
    if (!DuplicateHandle(GetCurrentProcess(), hOutputWrite,
                         GetCurrentProcess(), &hErrorWrite, 0,
                         TRUE, DUPLICATE_SAME_ACCESS))
    {
        OC_WARNING("[RuntimeCompiler] Failed to duplicate error output redirection pipe\n");
//        goto ERROR_EXIT;
    }
    si.hStdError = hErrorWrite;


    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (si.hStdOutput)
    {
        if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp,
                             GetCurrentProcess(),
                             &m_CmdProcessOutputRead, // Address of new handle.
                             0, FALSE, // Make it uninheritable.
                             DUPLICATE_SAME_ACCESS))
        {
            OC_WARNING("[RuntimeCompiler] Failed to duplicate output read pipe\n");
//            goto ERROR_EXIT;
        }
        CloseHandle(hOutputReadTmp);
        hOutputReadTmp = NULL;
    }


    HANDLE hInputRead, hInputWriteTmp;
    // Create a pipe for the child process's STDIN.
    if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 4096))
    {
        OC_WARNING("[RuntimeCompiler] Failed to create input pipes\n");
//        goto ERROR_EXIT;
    }
    si.hStdInput = hInputRead;

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (si.hStdOutput)
    {
        if (!DuplicateHandle(GetCurrentProcess(), hInputWriteTmp,
                             GetCurrentProcess(),
                             &m_CmdProcessInputWrite, // Address of new handle.
                             0, FALSE, // Make it uninheritable.
                             DUPLICATE_SAME_ACCESS))
        {
            OC_WARNING("[RuntimeCompiler] Failed to duplicate input write pipe\n");
//            goto ERROR_EXIT;
        }
    }


    const wchar_t* pCommandLine = L"cmd /q /K @PROMPT $";
    //CreateProcessW won't accept a const pointer, so copy to an array
    wchar_t pCmdLineNonConst[1024];
    wcscpy_s(pCmdLineNonConst, pCommandLine);
    CreateProcessW(
        NULL,				//__in_opt     LPCTSTR lpApplicationName,
        pCmdLineNonConst,			//__inout_opt  LPTSTR lpCommandLine,
        NULL,				//__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
        NULL,				//__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
        TRUE,				//__in         BOOL bInheritHandles,
        0,				//__in         DWORD dwCreationFlags,
        NULL,				//__in_opt     LPVOID lpEnvironment,
        NULL,				//__in_opt     LPCTSTR lpCurrentDirectory,
        &si,				//__in         LPSTARTUPINFO lpStartupInfo,
        &m_CmdProcessInfo				//__out        LPPROCESS_INFORMATION lpProcessInformation
    );

    //launch threaded read.
    _beginthread(ReadAndHandleOutputThread, 0, this); //this will exit when process for compile is closed


ERROR_EXIT:
    if( hOutputReadTmp )
    {
        CloseHandle( hOutputReadTmp );
    }
    if( hOutputWrite )
    {
        CloseHandle(hOutputWrite);
    }
    if( hErrorWrite )
    {
        CloseHandle( hErrorWrite );
    }
}


void CmdProcess::WriteInput( std::string& input )
{
    DWORD nBytesWritten;
    DWORD length = (DWORD)input.length();
    WriteFile( m_CmdProcessInputWrite , input.c_str(), length, &nBytesWritten, NULL);
}

void CmdProcess::CleanupProcessAndPipes()
{
    // do not reset m_bIsComplete and other members here, just process and pipes and
    if( !m_PathTempCLCommandFile.empty() && fs::exists(m_PathTempCLCommandFile) )
    {
        fs::remove(m_PathTempCLCommandFile);
        m_PathTempCLCommandFile.clear();
    }

    if( m_CmdProcessInfo.hProcess )
    {
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

CmdProcess::~CmdProcess()
{
    CleanupProcessAndPipes();
}


}// namespace vision::inline hotfix
