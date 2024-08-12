//
// Created by Zero on 2024/8/6.
//

#include "compiler.h"

#include "platform_win/cmd_process.h"
#include "platform_win/visual_studio_utils.h"

namespace vision::inline hotfix {

using namespace ocarina;

class CompilerVisualStudio : public Compiler {
private:
    CmdProcess cmd_process_;
    fs::path vs_path_;

public:
    void init() noexcept override {
        vector<VSVersionInfo> vec = GetPathsOfVisualStudioInstalls();
        CompileOptions op;
        vs_path_ = vec.at(0).Path;
        if (!fs::exists(FileInspector::intermediate_path())) {
            fs::create_directory(FileInspector::intermediate_path());
        }
        clear_directory(FileInspector::intermediate_path());
    }
    [[nodiscard]] string get_object_file_extension() const noexcept override {
        return "obj";
    }

    [[nodiscard]] static string assemble_compile_cmd(const fs::path &src_file, const fs::path &output_path) noexcept {
        return ocarina::format(R"(/nologo /Z7 /FC /utf-8 /MDd /Od /MP /Fo "{}" /D WIN32 /EHa /c "{}")",
                               output_path.string(), src_file.string());
    }

    void compile(const vision::CompileOptions &options,
                 const FileInspector::Module &module) noexcept override {
        fs::path module_path = FileInspector::intermediate_path() / module.name;
        if (!fs::exists(module_path)) {
            fs::create_directory(module_path);
        }
        for (const auto &file : module.files) {
            if (file.path.extension() != ".cpp") {
                continue;
            }

            std::string cmdSetParams = "\"" + vs_path_.string() + "Vcvarsall.bat\" x86_amd64\n";

            cmd_process_.InitialiseProcess();
            cmd_process_.WriteInput(cmdSetParams);

            string cmd = assemble_compile_cmd(file.path, module_path);
            fs::path cmd_fn = (module_path / file.path.stem()).string() + ".tmp";
            std::ofstream cmd_file(cmd_fn);
            cmd_file <<cmd;
            cout << cmd << endl;
            cmd_file.close();
        }
    }
};

UP<Compiler> Compiler::create() noexcept {
    auto ret = make_unique<CompilerVisualStudio>();
    ret->init();
    return ret;
}

}// namespace vision::inline hotfix