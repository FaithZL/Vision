//
// Created by Zero on 09/09/2022.
//

#include "cli_parser.h"
#include "core/logging.h"

namespace vision {

using namespace ocarina;

CLIParser::CLIParser(int argc, char **argv)
    : argc_(argc), argv_(argv),
      cli_options_{ocarina::fs::path{argv[0]}.filename().string()} {
    init(argc, argv);
}

void CLIParser::init(int argc, char **argv) {
    cli_options_.add_options(
        "Renderer",
        {{"d, device", "Select compute device:",
          cxxopts::value<std::string>()->default_value("cuda")},
         {"p, pipeline", "Select render pipeline:",
          cxxopts::value<std::string>()->default_value("fixed")},
         {"r, runtime-dir", "Specify runtime directory",
          cxxopts::value<fs::path>()->default_value(
              fs::canonical(argv[0]).parent_path().parent_path().string())},
         {"w, working-dir", "Specify working directory",
          cxxopts::value<fs::path>()->default_value(
              fs::canonical(fs::current_path()).string())},
         {"output-dir", "The output file path, default: the same directory as the scene description files",
          cxxopts::value<std::string>()->default_value("")},
         {"c, clear-cache", "Clear cached", cxxopts::value<bool>()},
         {"m, mode", "run mode: cli or gui", cxxopts::value<std::string>()->default_value("cli")},
         {"t, thread-num", "the num of threads to render", cxxopts::value<std::string>()->default_value("0")},
         {"s, scene", "The scene to render,file name end with json or scene supported by assimp", cxxopts::value<std::string>()},
         {"o, output", "The output rad_collector file path, output file will be saved as EXR format\n."
                       "Alternatively, you can specify the output file path in scene description \n"
                       "file(-s|--scene), this option will override the setting in scene file",
          cxxopts::value<std::string>()->default_value("")},
         {"positional", "Specify input file", cxxopts::value<std::string>()},
         {"denoise", "Denoise using default denoiser"},
         {"h, help", "Print usage"}});
}

const cxxopts::ParseResult &CLIParser::_parse_result() const noexcept {
    if (!parsed_cli_options_.has_value()) {
        cli_options_.parse_positional("positional");
        parsed_cli_options_.emplace(
            cli_options_.parse(const_cast<int &>(argc_), const_cast<const char **&>(argv_)));
    }
    return *parsed_cli_options_;
}

void CLIParser::print_help() const noexcept {
    cout << cli_options_.help() << endl;
}

fs::path CLIParser::working_dir() const noexcept {
    return fs::canonical(_parse_result()["working-dir"].as<fs::path>());
}

fs::path CLIParser::runtime_dir() const noexcept {
    return fs::canonical(_parse_result()["runtime-dir"].as<fs::path>());
}

fs::path CLIParser::scene_path() const noexcept {
    return scene_file().parent_path();
}

fs::path CLIParser::output_dir() const noexcept {
    return fs::canonical(_parse_result()["output-dir"].as<std::string>());
}

fs::path CLIParser::scene_file() const noexcept {
    return fs::canonical(_parse_result()["scene"].as<std::string>());
}

bool CLIParser::clear_cache() const noexcept {
    return _parse_result()["clear-cache"].as<bool>();
}

string CLIParser::backend() const noexcept {
    return _parse_result()["device"].as<std::string>();
}

string CLIParser::pipeline() const noexcept {
    return _parse_result()["pipeline"].as<string>();
}

string CLIParser::cli_positional_option() const {
    return _parse_result()["positional"].as<string>();
}

}// namespace vision