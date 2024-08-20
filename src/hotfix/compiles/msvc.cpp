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
//        vector<VSVersionInfo> vec = GetPathsOfVisualStudioInstalls();
//        env_path_ = vec.at(0).Path;
        env_path_ = installation_directory() / bat_dir;
        if (!fs::exists(FileInspector::intermediate_path())) {
            fs::create_directory(FileInspector::intermediate_path());
        }
        auto aaa = installation_directory();

        clear_directory(FileInspector::intermediate_path());
    }

    [[nodiscard]] fs::path installation_directory() noexcept override {
        return parent_path(cli_path(), 8);
    }

    [[nodiscard]] string get_object_file_extension() const noexcept override {
        return "obj";
    }

    void setup_environment() const {
        std::string cmdSetParams = "\"" + env_path_.string() + "Vcvarsall.bat\" x86_amd64\n";
        cmd_process_.WriteInput(cmdSetParams);
        cmd_process_.WriteInput(std::string("chcp 65001\n"));
    }

    [[nodiscard]] static string assemble_compile_cmd(const fs::path &src_file,
                                                     const fs::path &output_path,
                                                     const BuildOptions &options) noexcept {
        string cmd = ocarina::format(R"(/nologo /Z7 /FC /utf-8 /MDd /Od /MP /DFMT_CONSTEVAL=constexpr -D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING /Fo"{}\\" /D WIN32 /EHa /c "{}")",
                                     output_path.string(), src_file.string());
        for (const auto &p : options.include_paths) {
            cmd += format(R"( /I "{}")", p.string());
        }
        cmd += " /std:c++20";
        return cmd;
    }

    void compile(const vision::BuildOptions &options,
                 const FileInspector::Module &module) noexcept override {
        fs::path module_path = FileInspector::intermediate_path() / module.name;
        if (!fs::exists(module_path)) {
            fs::create_directory(module_path);
        }
        cmd_process_.InitialiseProcess();
        setup_environment();
        for (const auto &file : module.files) {
            if (file.path.extension() != ".cpp") {
                continue;
            }

            string cmd = assemble_compile_cmd(file.path, module_path, options);
            fs::path cmd_fn = (module_path / file.path.stem()).string() + ".tmp";
            std::ofstream cmd_file(cmd_fn);
            cmd_file << cmd;
            cmd_file.close();
            string cmd_to_send = "cl @" + cmd_fn.string();
            cmd_to_send += "\necho ";
            cmd_to_send += string(c_CompletionToken) + "\n";
            cmd_process_.WriteInput(cmd_to_send);
        }
    }
};

}// namespace vision::inline hotfix

VS_EXPORT_API vision::hotfix::MSVCompiler *create() {
    return ocarina::new_with_allocator<vision::hotfix::MSVCompiler>();
}

VS_EXPORT_API void destroy(vision::hotfix::MSVCompiler *obj) {
    ocarina::delete_with_allocator(obj);
}