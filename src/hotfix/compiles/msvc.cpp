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

    void setup_environment() const override {
        std::string cmdSetParams = "\"" + env_path_.string() + "Vcvarsall.bat\" x86_amd64\n";
        cmd_process_.write_input(cmdSetParams);
        cmd_process_.write_input(std::string("chcp 65001\n"));
        cmd_process_.change_directory(BuildSystem::directory());
    }

    void compile(const CompileOptions &options) noexcept override;

    void link(const vision::LinkOptions &options, const FileInspector::Target &target,
              const string &extension_objs) noexcept override;
};

void MSVCompiler::compile(const vision::CompileOptions &options) noexcept {
    /// defines includes flags obj cpp
    static constexpr string_view cmd_template = "cl /nologo /TP {} {} {} /Fo{} -c {}";
    string cmd = ocarina::format(cmd_template, options.defines, options.includes, options.flags,
                                 options.dst_fn.string(), options.src_fn.string());
    cmd_process_.write_input(cmd);
}

void MSVCompiler::link(const vision::LinkOptions &options,
                       const FileInspector::Target &target,
                       const string &extension_objs) noexcept {
    static constexpr string_view cmd_template = "link {} /out:{} /implib:{} /pdb:{} /dll {} {}";
    string link_cmd = ocarina::format(cmd_template, options.obj_files_string() + extension_objs,
                                      target.dll_path().string(),
                                      target.lib_path().string(),
                                      target.pdb_path().string(),
                                      options.link_flags,
                                      options.link_libraries);
    cmd_process_.write_input(link_cmd);
}

}// namespace vision::inline hotfix

VS_EXPORT_API vision::hotfix::MSVCompiler *create() {
    return ocarina::new_with_allocator<vision::hotfix::MSVCompiler>();
}

VS_EXPORT_API void destroy(vision::hotfix::MSVCompiler *obj) {
    ocarina::delete_with_allocator(obj);
}