//
// Created by Zero on 2024/8/6.
//

#include "hotfix/compiler.h"
#include "core/vs_header.h"
#include "hotfix/build_system.h"
#include "hotfix/cmd_process.h"

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
        cmd_process_.initialise();
    }

    ~MSVCompiler() override {
        cmd_process_.cleanup_process();
    }

    [[nodiscard]] fs::path installation_directory() noexcept override {
        return parent_path(cli_path(), 8);
    }

    [[nodiscard]] string get_object_file_extension() const noexcept override {
        return "obj";
    }

    void setup_environment() const override {
        std::string cmdSetParams = "\"" + env_path_.string() + "Vcvarsall.bat\" x86_amd64\n";
        cmd_process_.write_input(cmdSetParams);
        cmd_process_.write_input(std::string("chcp 65001\n"));
        cmd_process_.change_directory(BuildSystem::directory());
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
        cmd_process_.write_input(cmd);
    }

    [[nodiscard]] static vector<string> assemble_link_cmds(const LinkOptions &options, const FileInspector::Target &target) noexcept;
    void link(const vision::LinkOptions &options, const FileInspector::Target &target) noexcept override;
};

vector<string> MSVCompiler::assemble_link_cmds(const LinkOptions &options,
                                      const FileInspector::Target &target) noexcept {
    /// pre link
    vector<string> ret;
    auto ccc = "link src\\hotfix\\test\\CMakeFiles\\vision-hotfix-test.dir\\demo.cpp.obj src\\hotfix\\test\\CMakeFiles\\vision-hotfix-test.dir\\test.cpp.obj  /out:bin\\vision-hotfix-test333.dll /implib:lib\\vision-hotfix-test.lib /pdb:bin\\vision-hotfix-test.pdb /dll /version:0.0 /machine:x64 /debug /INCREMENTAL  /DEF:src\\hotfix\\test\\CMakeFiles\\vision-hotfix-test.dir\\.\\exports.def  lib\\vision-hotfix.lib  lib\\ocarina.lib  lib\\ocarina-dsl.lib  lib\\ocarina-rhi.lib  lib\\ocarina-GUI.lib  lib\\ocarina-generator.lib  lib\\ocarina-ast.lib  lib\\ocarina-util.lib  lib\\ocarina-core.lib  lib\\ocarina-math.lib  lib\\ocarina-ext.lib  lib\\EASTL.lib  lib\\spdlogd.lib  lib\\ocarina-imgui.lib  lib\\glfw3.lib  lib\\ocarina-ext-stb.lib  lib\\ocarina-ext-tinyexr.lib  dbghelp.lib  kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib";

    /// obj out implib pdb link_flags dependency
    static constexpr string_view cmd_template = "link {} /out:{} /implib:{} /pdb:{} /dll {} {}";
    string link_cmd = ocarina::format(cmd_template, options.obj_files_string(),
                                      "bin\\vision-hotfix-test333.dll",
                                      "lib\\vision-hotfix-test123.lib",
                                      "bin\\vision-hotfix-test567.pdb",
                                      options.link_flags,
                                      options.link_libraries);

    string cmd = options.pre_link + "&& " + link_cmd;
    ret.push_back(cmd);

    return ret;
}

void MSVCompiler::link(const vision::LinkOptions &options,
                       const FileInspector::Target &target) noexcept {
    auto cmds = assemble_link_cmds(options, target);
    for (const auto &item : cmds) {
        cmd_process_.write_input(item);
    }
}

}// namespace vision::inline hotfix

VS_EXPORT_API vision::hotfix::MSVCompiler *create() {
    return ocarina::new_with_allocator<vision::hotfix::MSVCompiler>();
}

VS_EXPORT_API void destroy(vision::hotfix::MSVCompiler *obj) {
    ocarina::delete_with_allocator(obj);
}