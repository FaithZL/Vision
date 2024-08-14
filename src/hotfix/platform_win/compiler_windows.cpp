//
// Created by Zero on 2024/8/6.
//

#include "hotfix/compiler.h"

#include "hotfix/cmd_process.h"
#include "visual_studio_utils.h"


namespace vision::inline hotfix {

using namespace ocarina;

class CompilerVisualStudio : public Compiler {
private:
    CmdProcess cmd_process_;
    fs::path vs_path_;

public:
    void init() noexcept override {
        vector<VSVersionInfo> vec = GetPathsOfVisualStudioInstalls();
        BuildOptions op;
        vs_path_ = vec.at(0).Path;
        if (!fs::exists(FileInspector::intermediate_path())) {
            fs::create_directory(FileInspector::intermediate_path());
        }
        clear_directory(FileInspector::intermediate_path());
    }
    [[nodiscard]] string get_object_file_extension() const noexcept override {
        return "obj";
    }

    void setup_environment() const {
        std::string cmdSetParams = "\"" + vs_path_.string() + "Vcvarsall.bat\" x86_amd64\n";
        cmd_process_.WriteInput(cmdSetParams);
        cmd_process_.WriteInput( std::string("chcp 65001\n") );
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

    void compile(const vision::CompileOptions &options,
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
            string cmd_to_send = "cl @" + cmd_fn.string() ;
            cmd_to_send += "\necho ";
            cmd_to_send += string(c_CompletionToken) + "\n";
            cmd_process_.WriteInput(cmd_to_send);
        }
    }
};

UP<Compiler> Compiler::create() noexcept {
    auto ret = make_unique<CompilerVisualStudio>();
    ret->init();
    return ret;
}

}// namespace vision::inline hotfix