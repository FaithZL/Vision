//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "cxxopts.hpp"
#include "core/stl.h"

namespace vision {

using namespace ocarina;

class CLIParser {
private:
    int argc_;
    char **argv_;
    mutable cxxopts::Options cli_options_;
    mutable std::optional<cxxopts::ParseResult> parsed_cli_options_;

private:
    const cxxopts::ParseResult &_parse_result() const noexcept;

public:
    CLIParser(int argc, char **argv);
    void init(int argc, char **argv);
    [[nodiscard]] string cli_positional_option() const;
    void print_help() const noexcept;
    [[nodiscard]] bool has_help_cmd() const noexcept { return _parse_result().count("help") > 0; }
    void try_print_help_and_exit() const noexcept {
        if (has_help_cmd()) {
            print_help();
            exit(0);
        }
    }
    [[nodiscard]] bool clear_cache() const noexcept;
    [[nodiscard]] string backend() const noexcept;
    [[nodiscard]] string pipeline() const noexcept;
    [[nodiscard]] fs::path scene_file() const noexcept;
    [[nodiscard]] fs::path working_dir() const noexcept;
    [[nodiscard]] fs::path runtime_dir() const noexcept;
    [[nodiscard]] fs::path scene_path() const noexcept;
    [[nodiscard]] fs::path output_dir() const noexcept;
};

}// namespace vision
