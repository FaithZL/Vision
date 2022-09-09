//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "cxxopts.hpp"
#include "ocarina/src/core/stl.h"

namespace vision {

using namespace ocarina;

class CLIParser {
private:
    int _argc;
    char **_argv;
    fs::path _run_dir;
    fs::path _work_dir;
    fs::path _res_dir;
    fs::path _in_dir;
    fs::path _scene_file;
    fs::path _output_dir;
    mutable cxxopts::Options _cli_options;
    mutable std::optional<cxxopts::ParseResult> _parsed_cli_options;

private:
    [[nodiscard]] fs::path _working_dir() noexcept;
    [[nodiscard]] fs::path _runtime_dir() noexcept;
    [[nodiscard]] fs::path _input_dir() noexcept;
    const cxxopts::ParseResult &_parse_result() const noexcept;

public:
    CLIParser(int argc, char **argv);
    void init(int argc, char **argv);
    [[nodiscard]] string cli_positional_option() const;
    void print_help();
    [[nodiscard]] bool clear_cache() noexcept;
    [[nodiscard]] string backend() noexcept;
    [[nodiscard]] fs::path working_path(const fs::path &name = {}) noexcept;
    [[nodiscard]] fs::path runtime_path(const fs::path &name = {}) noexcept;
    [[nodiscard]] fs::path input_path(const fs::path &name = {}) noexcept;
    [[nodiscard]] fs::path scene_file() noexcept;
    [[nodiscard]] fs::path scene_path() noexcept;
    [[nodiscard]] fs::path output_dir() noexcept;
};

}// namespace vision
