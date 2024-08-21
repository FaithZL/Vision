//
// Created by Zero on 2024/7/26.
//

#pragma once

#include "core/stl.h"
#include "file_inspector.h"
#include "util/file_manager.h"
#include "core/dynamic_module.h"
#include "build_rules.h"

namespace vision::inline hotfix {

class Compiler {
public:
    using Creator = Compiler *();
    using Deleter = void(Compiler *);
    using Handle = unique_ptr<Compiler, Deleter *>;

public:
    virtual void compile(const CompileOptions &options) noexcept = 0;
    virtual void link(const LinkOptions &options, const FileInspector::Target &target) noexcept = 0;
    [[nodiscard]] virtual string get_object_file_extension() const noexcept = 0;
    [[nodiscard]] static fs::path cli_path() noexcept;
    [[nodiscard]] virtual fs::path installation_directory() noexcept = 0;
    virtual void setup_environment() const {}
    [[nodiscard]] static Handle create(const string &name);
    [[nodiscard]] static Handle create();
    virtual ~Compiler() = default;
};

}// namespace vision::inline hotfix