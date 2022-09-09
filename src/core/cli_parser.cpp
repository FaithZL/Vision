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

const cxxopts::ParseResult &CLIParser::_parse_result() const noexcept  {
    if (!_parsed_cli_options.has_value()) {
        _cli_options.parse_positional("positional");
        _parsed_cli_options.emplace(
            _cli_options.parse(const_cast<int &>(_argc), const_cast<const char **&>(_argv)));
    }
    return *_parsed_cli_options;
}

void CLIParser::print_help() {
    cout << _cli_options.help() << endl;
}

fs::path CLIParser::_working_dir() noexcept {
    if (_work_dir.empty()) {
        _work_dir = fs::canonical(_parse_result()["working-dir"].as<fs::path>());
        OC_ERROR_IF(!fs::exists(_work_dir) || !fs::is_directory(_work_dir),
                              "Invalid working directory: ", _work_dir);
        fs::current_path(_work_dir);
        OC_INFO("Working directory: ", _work_dir);
    }
    return _work_dir;
}

fs::path CLIParser::_runtime_dir() noexcept {
    if (_run_dir.empty()) {
        _run_dir = fs::canonical(_parse_result()["runtime-dir"].as<fs::path>());
        OC_ERROR_IF(!fs::exists(_run_dir) || !fs::is_directory(_run_dir),
                              "Invalid runtime directory: ", _run_dir);
        OC_INFO("Runtime directory: ", _run_dir);
    }
    return _run_dir;
}

fs::path CLIParser::_input_dir() noexcept {
    if (_in_dir.empty()) {
        if (_parse_result().count("positional") == 0u) {
            OC_WARNING("No positional CLI argument given, setting input directory to working directory: ",
                             _working_dir());
        } else {
            _in_dir = fs::canonical(cli_positional_option()).parent_path();
            OC_ERROR_IF(!fs::exists(_in_dir) || !fs::is_directory(_in_dir),
                                  "Invalid input directory: ", _in_dir);
            OC_INFO("Input directory: ", _in_dir);
        }
    }
    return _in_dir;
}

fs::path CLIParser::input_path(const fs::path &name) noexcept {
    return _input_dir() / name;
}

fs::path CLIParser::working_path(const fs::path &name) noexcept {
    return _working_dir() / name;
}

fs::path CLIParser::runtime_path(const fs::path &name) noexcept {
    return _runtime_dir() / name;
}

fs::path CLIParser::scene_path() noexcept {
    return scene_file().parent_path();
}

fs::path CLIParser::output_dir() noexcept {
    if (_output_dir.empty()) {
        _output_dir = fs::canonical(_parse_result()["output-dir"].as<std::string>());
        if (_output_dir.empty()) {
            _output_dir = scene_path();
        }
    }
    return _output_dir;
}

fs::path CLIParser::scene_file() noexcept {
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