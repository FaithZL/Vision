//
// Created by Zero on 2024/8/13.
//

#pragma once

#include "core/stl.h"
#include "core/logging.h"
#include "util/file_manager.h"
#include "core/dynamic_module.h"
#include "module_interface.h"

namespace vision::inline hotfix {

using namespace ocarina;

struct CompileOptions {
    fs::path src_fn;
    fs::path dst_fn;
    string defines;
    string flags;
    string includes;
};

struct LinkOptions {
    string compile_flags;
    string link_flags;
    string pre_link;
    vector<fs::path> obj_files;
    vector<fs::path> all_libraries;
    string link_libraries;
    fs::path target_file;
    fs::path target_pdb;
    fs::path target_implib;
    [[nodiscard]] string obj_files_string() const noexcept {
        string ret;
        for (const auto &item : obj_files) {
            ret += item.string() + " ";
        }
        return ret;
    }
};

class BuildRules {
public:
    using Creator = BuildRules *();
    using Deleter = void(BuildRules *);
    using Handle = unique_ptr<BuildRules, Deleter *>;

protected:
    map<string, CompileOptions> compile_map_;
    map<string, LinkOptions> link_map_;
    map<string_view, string> cpp_to_obj_;

public:
    BuildRules() = default;
    [[nodiscard]] CompileOptions compile_options(const string &src_fn) const noexcept {
        return compile_map_.at(src_fn);
    }
    [[nodiscard]] LinkOptions link_options(const string &target_fn) const noexcept {
        return link_map_.at(target_fn);
    }
    [[nodiscard]] string obj_path(string_view cpp_path) const noexcept {
        return cpp_to_obj_.at(cpp_path);
    }
    template<typename ...Args>
    [[nodiscard]] vector<string> obj_paths(Args &&...args) const noexcept {
        vector<string> ret;
        ret.reserve(sizeof...(Args));
        (ret.push_back(obj_path(OC_FORWARD(args))),...);
        return ret;
    }
    virtual void parse(const string &content) = 0;
    [[nodiscard]] static Handle create(const string &name = "ninja") {
        string plugin_name = ocarina::format("vision-hotfix-rules_parser-{}.dll", name);
        auto module = ocarina::FileManager::instance().obtain_module(plugin_name);
        auto creator = module->function<BuildRules::Creator *>("create");
        auto deleter = module->function<BuildRules::Deleter *>("destroy");
        return Handle{creator(), deleter};
    }
    virtual ~BuildRules() = default;
};

}// namespace vision::inline hotfix