//
// Created by Zero on 09/09/2022.
//

#include "cli_parser.h"
#include "ocarina/src/core/logging.h"

namespace vision {

using namespace ocarina;

CLIParser::CLIParser(int argc, char **argv)
    : _argc(argc), _argv(argv),
      _cli_options{ocarina::fs::path{argv[0]}.filename().string()} {
    init(argc, argv);
}

void CLIParser::init(int argc, char **argv) {
    _cli_options.add_options(
        "Renderer",
        {{"d, device", "Select compute device: cuda",
          cxxopts::value<std::string>()->default_value("cuda")},
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
         {"o, output", "The output film file path, output file will be saved as EXR format\n."
                       "Alternatively, you can specify the output file path in scene description \n"
                       "file(-s|--scene), this option will override the setting in scene file",
          cxxopts::value<std::string>()->default_value("")},
         {"positional", "Specify input file", cxxopts::value<std::string>()},
         {"denoise", "Denoise using default denoiser"},
         {"h, help", "Print usage"}});
}

const cxxopts::ParseResult &CLIParser::_parse_result() const noexcept {
    if (!_parsed_cli_options.has_value()) {
        _cli_options.parse_positional("positional");
        _parsed_cli_options.emplace(
            _cli_options.parse(const_cast<int &>(_argc), const_cast<const char **&>(_argv)));
    }
    return *_parsed_cli_options;
}

void CLIParser::print_help() const noexcept {
    cout << _cli_options.help() << endl;
}

fs::path CLIParser::working_dir() noexcept {
    return fs::canonical(_parse_result()["working-dir"].as<fs::path>());
}

fs::path CLIParser::runtime_dir() noexcept {
    return fs::canonical(_parse_result()["runtime-dir"].as<fs::path>());
}

fs::path CLIParser::scene_path() const noexcept {
    return scene_file().parent_path();
}

fs::path CLIParser::output_dir() noexcept {
    return fs::canonical(_parse_result()["output-dir"].as<std::string>());
}

fs::path CLIParser::scene_file() const noexcept {
    return fs::canonical(_parse_result()["scene"].as<std::string>());
}

bool CLIParser::clear_cache() noexcept {
    return _parse_result()["clear-cache"].as<bool>();
}

string CLIParser::backend() noexcept {
    return _parse_result()["device"].as<std::string>();
}

string CLIParser::cli_positional_option() const {
    return _parse_result()["positional"].as<string>();
}

}// namespace vision