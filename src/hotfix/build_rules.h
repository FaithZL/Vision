//
// Created by Zero on 2024/8/13.
//

#pragma once

#include "core/stl.h"
#include "core/logging.h"
#include "util/file_manager.h"
#include "core/dynamic_module.h"

namespace vision::inline hotfix {

using namespace ocarina;

struct CompileOptions {
    vector<string> defines;
    vector<string> flags;
    vector<fs::path> includes;
    fs::path src;
    fs::path dst;
};

struct LinkOptions {
    vector<string> compile_flags;
    vector<fs::path> obj_files;
    vector<fs::path> link_libraries;
    fs::path target_file;
    fs::path target_implib;
};

class BuildRules {
public:
    using Creator = BuildRules *();
    using Deleter = void(BuildRules *);
    using Handle = unique_ptr<BuildRules, Deleter *>;

protected:
    vector<CompileOptions> compiles_;
    vector<LinkOptions> links_;

public:
    BuildRules() = default;
    virtual void parse(const string &content) = 0;
    [[nodiscard]] static Handle create(const string &name) {
        string plugin_name = ocarina::format("vision-hotfix-rules_parser-{}.dll", name);
        auto module = ocarina::FileManager::instance().obtain_module(plugin_name);
        auto creator = module->function<BuildRules::Creator *>("create");
        auto deleter = module->function<BuildRules::Deleter *>("destroy");
        return Handle{creator(), deleter};
    }
    virtual ~BuildRules() = default;
};

}// namespace vision::inline hotfix