//
// Created by Zero on 2024/8/5.
//

#pragma once

#include <windows.h>

namespace vision::inline hotfix {

struct VSVersionInfo {
    fs::path path;
};

struct VSKey {
    const char *keyName;
    const char *pathToAdd;
    HKEY key;
};

struct VSVersionDiscoveryInfo {
    const char *versionName;
    const char *versionNextName;
    int versionKey;
    bool tryVSWhere;
};

void get_visualstudio_paths(vector<VSVersionInfo> *lst) {
    VSKey VS_KEYS[] = {{R"(SOFTWARE\Microsoft\VisualStudio\SxS\VC7)", "", nullptr},
                       {R"(SOFTWARE\Microsoft\VisualStudio\SxS\VS7)", R"(VC\Auxiliary\Build\)", nullptr}};
    int NUMVSKEYS = sizeof(VS_KEYS) / sizeof(VSKey);

    VSVersionDiscoveryInfo VS_DISCOVERY_INFO[] = {{"8.0", "9.0", 0, false},
                                                  {"9.0", "10.0", 0, false},
                                                  {"10.0", "11.0", 0, false},
                                                  {"11.0", "12.0", 0, false},
                                                  {"12.0", "13.0", 0, false},
                                                  {"14.0", "15.0", 0, false},
                                                  {"15.0", "16.0", 1, true},
                                                  {"16.0", "17.0", 1, true},
                                                  {"17.0", "18.0", 1, true}};

    int NUMNAMESTOCHECK = sizeof(VS_DISCOVERY_INFO) / sizeof(VSVersionDiscoveryInfo);

    int startVersion = NUMNAMESTOCHECK - 1;

    const uint32_t MSCVERSION = _MSC_VER;
    bool bMSCVersionFound = true;
    switch (MSCVERSION) {
        case 1400://VS 2005
            startVersion = 0;
            break;
        case 1500://VS 2008
            startVersion = 1;
            break;
        case 1600://VS 2010
            startVersion = 2;
            break;
        case 1700://VS 2012
            startVersion = 3;
            break;
        case 1800://VS 2013
            startVersion = 4;
            break;
        case 1900://VS 2015
            startVersion = 5;
            break;
        case 1910://VS 2017
        case 1911://VS 2017
        case 1912://VS 2017
        case 1913://VS 2017
        case 1914://VS 2017
        case 1915://VS 2017
        case 1916://VS 2017
            startVersion = 6;
            break;
        case 1920:// VS 2019
        case 1921:// VS 2019
        case 1922:// VS 2019
        case 1923:// VS 2019
        case 1924:// VS 2019
        case 1925:// VS 2019
        case 1926:// VS 2019
        case 1927:// VS 2019
        case 1928:// VS 2019
        case 1929:// VS 2019
            startVersion = 7;
            break;
        case 1930:// VS 2022
        case 1931:// VS 2022
            startVersion = 8;
            break;
        default:
            bMSCVersionFound = false;
            OC_INFO_FORMAT("WARNING: VS Compiler with _MSC_VER {} potentially not supported. Defaulting to version {}.\n",
                           MSCVERSION, VS_DISCOVERY_INFO[startVersion].versionName);
    }

    char value[MAX_PATH];
    DWORD size = MAX_PATH;

    for (int i = 0; i < NUMVSKEYS; ++i) {
        LONG retKeyVal = RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,        //__in        HKEY hKey,
            VS_KEYS[i].keyName,        //__in_opt    LPCTSTR lpSubKey,
            0,                         //__reserved  DWORD ulOptions,
            KEY_READ | KEY_WOW64_32KEY,//__in        REGSAM samDesired,
            &VS_KEYS[i].key            //__out       PHKEY phkResult
        );
    }

    int loopCount = 1;
    if (startVersion != NUMNAMESTOCHECK - 1) {
        // we potentially need to restart search from top
        loopCount = 2;
    }

    for (int loop = 0; loop < loopCount; ++loop) {
        for (int i = startVersion; i >= 0; --i) {
            VSVersionDiscoveryInfo vsinfo = VS_DISCOVERY_INFO[i];
            VSKey vskey = VS_KEYS[vsinfo.versionKey];

            if (vsinfo.tryVSWhere) {
                CmdProcess cmdProc;
                cmdProc.init_process();
                cmdProc.store_cmd_output = true;
                cmdProc.cmd_output = "";
                std::string maxVersion;
                if (!bMSCVersionFound && i == startVersion) {
                    // open ended max version so we find the latest
                    maxVersion = "";
                } else {
                    maxVersion = vsinfo.versionNextName;
                }
                std::string vsWhereQuery = "\"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere\""
                                           " -version [" +
                                           std::string(vsinfo.versionName) + "," + maxVersion + ") "// [min,max) format for version names
                                                                                                " -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
                                                                                                " -property installationPath"
                                                                                                "\nexit\n";
                cmdProc.write_input(vsWhereQuery);
                WaitForSingleObject(cmdProc.cmd_process_info.hProcess, 2000);// max 2 secs
                // get the first non-empty substring
                size_t start = cmdProc.cmd_output.find_first_not_of("\r\n", 0);
                if (start != std::string::npos) {
                    size_t end = cmdProc.cmd_output.find_first_of("\r\n", start);
                    if (end == std::string::npos) {
                        end = cmdProc.cmd_output.length();
                    }
                    fs::path path = cmdProc.cmd_output.substr(start, end - start);
                    if (path.string().length() && fs::exists(path)) {
                        VSVersionInfo vInfo;
                        vInfo.path = path.string() / vskey.pathToAdd;
                        lst->push_back(vInfo);
                        continue;
                    }
                }
            }

            LONG retVal = RegQueryValueExA(
                vskey.key,         //__in         HKEY hKey,
                vsinfo.versionName,//__in_opt     LPCTSTR lpValueName,
                nullptr,              //__reserved   LPDWORD lpReserved,
                nullptr,              //__out_opt    LPDWORD lpType,
                (LPBYTE)value,     //__out_opt    LPBYTE lpData,
                &size              //__inout_opt  LPDWORD lpcbData
            );
            if (ERROR_SUCCESS == retVal) {
                VSVersionInfo vInfo;
                vInfo.path = value;
                vInfo.path += vskey.pathToAdd;
                lst->push_back(vInfo);
            }
        }
    }
}
}// namespace vision::inline hotfix