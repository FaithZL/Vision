//
// Created by Zero on 2024/8/6.
//

#include "compiler.h"
#include <windows.h>
#include "cmd_process.h"

namespace vision::inline hotfix {

using namespace ocarina;

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
}

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