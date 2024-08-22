//
// Created by Zero on 2024/8/6.
//

#include "hotfix/compiler.h"
#include "core/vs_header.h"
#include "hotfix/cmd_process.h"
#include "visual_studio_utils.h"

namespace vision::inline hotfix {
using namespace ocarina;

class MSVCompiler : public Compiler {
private:
    static constexpr auto bat_dir = R"(VC\Auxiliary\Build\)";
    CmdProcess cmd_process_;
    fs::path env_path_;

public:
    MSVCompiler() {
        env_path_ = installation_directory() / bat_dir;
        if (!fs::exists(FileInspector::intermediate_path())) {
            fs::create_directory(FileInspector::intermediate_path());
        }
        clear_directory(FileInspector::intermediate_path());
        cmd_process_.InitialiseProcess();
    }

    ~MSVCompiler() override {
        cmd_process_.CleanupProcessAndPipes();
    }

    [[nodiscard]] fs::path installation_directory() noexcept override {
        return parent_path(cli_path(), 8);
    }

    [[nodiscard]] string get_object_file_extension() const noexcept override {
        return "obj";
    }

    void setup_environment() const override {
        std::string cmdSetParams = "\"" + env_path_.string() + "Vcvarsall.bat\" x86_amd64\n";
        cmd_process_.WriteInput(cmdSetParams);
        cmd_process_.WriteInput(std::string("chcp 65001\n"));
    }

    [[nodiscard]] static string add_complete_flag(const string &cmd) noexcept {
        return cmd + ocarina::format("\n echo {} \n", c_CompletionToken);
    }

    [[nodiscard]] static string assemble_compile_cmd(const CompileOptions &options) noexcept {
        /// defines includes flags obj cpp
        static constexpr string_view cmd_template = "cl /nologo /TP {} {} {} /Fo{} -c {}";
        string cmd = ocarina::format(cmd_template, options.defines, options.includes, options.flags,
                                     options.dst_fn.string(), options.src_fn.string());
        return cmd;
    }

    void compile(const CompileOptions &options) noexcept override {
        string cmd = assemble_compile_cmd(options);
        cmd = add_complete_flag(cmd);
        cmd_process_.WriteInput(cmd);
    }

    [[nodiscard]] static string assemble_link_cmd(const LinkOptions &options, const FileInspector::Target &target) noexcept;
    void link(const vision::LinkOptions &options, const FileInspector::Target &target) noexcept override;
};

string MSVCompiler::assemble_link_cmd(const LinkOptions &options,
                                      const FileInspector::Target &target) noexcept {
    /// pre link
    static constexpr string_view cmd_template = "{}";
    string cmd;
    return string(R"(C:\WINDOWS\system32\cmd.exe /C "C:\WINDOWS\system32\cmd.exe /C ""C:\Program Files\JetBrains\CLion 2024.2\bin\cmake\win\x64\bin\cmake.exe" -E __create_def D:\work\renderers\Vision\cmake-build-relwithdebinfo\src\hotfix\test\CMakeFiles\vision-hotfix-test.dir\.\exports.def D:\work\renderers\Vision\cmake-build-relwithdebinfo\src\hotfix\test\CMakeFiles\vision-hotfix-test.dir\.\exports.def.objs && cd D:\work\renderers\Vision\cmake-build-relwithdebinfo" && "C:\Program Files\JetBrains\CLion 2024.2\bin\cmake\win\x64\bin\cmake.exe" -E vs_link_dll --intdir=src\hotfix\test\CMakeFiles\vision-hotfix-test.dir --rc=C:\PROGRA~2\WI3CF2~1\10\bin\100226~1.0\x64\rc.exe --mt=C:\PROGRA~2\WI3CF2~1\10\bin\100226~1.0\x64\mt.exe --manifests  -- C:\PROGRA~1\MICROS~1\2022\COMMUN~1\VC\Tools\MSVC\1441~1.341\bin\Hostx64\x64\link.exe  src\hotfix\test\CMakeFiles\vision-hotfix-test.dir\demo.cpp.obj src\hotfix\test\CMakeFiles\vision-hotfix-test.dir\test.cpp.obj  /out:bin\vision-hotfix-test.dll /implib:lib\vision-hotfix-test.lib /pdb:bin\vision-hotfix-test.pdb /dll /version:0.0 /machine:x64 /debug /INCREMENTAL  /DEF:src\hotfix\test\CMakeFiles\vision-hotfix-test.dir\.\exports.def  lib\vision-hotfix.lib  lib\ocarina.lib  lib\ocarina-dsl.lib  lib\ocarina-rhi.lib  lib\ocarina-GUI.lib  lib\ocarina-generator.lib  lib\ocarina-ast.lib  lib\ocarina-util.lib  lib\ocarina-core.lib  lib\ocarina-math.lib  lib\ocarina-ext.lib  lib\EASTL.lib  lib\spdlog.lib  lib\ocarina-imgui.lib  lib\glfw3.lib  lib\ocarina-ext-stb.lib  lib\ocarina-ext-tinyexr.lib  dbghelp.lib  kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib && cd .")");
}

void MSVCompiler::link(const vision::LinkOptions &options,
                       const FileInspector::Target &target) noexcept {
    string cmd = assemble_link_cmd(options, target);
    cmd += "\n echo \n _COMPLETION_TOKEN_";
//    cmd_process_.WriteInput(cmd);
}

}// namespace vision::inline hotfix

VS_EXPORT_API vision::hotfix::MSVCompiler *create() {
    return ocarina::new_with_allocator<vision::hotfix::MSVCompiler>();
}

VS_EXPORT_API void destroy(vision::hotfix::MSVCompiler *obj) {
    ocarina::delete_with_allocator(obj);
}