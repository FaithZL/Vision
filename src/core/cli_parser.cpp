//
// Created by Zero on 09/09/2022.
//

#include "cli_parser.h"

namespace vision {

using namespace ocarina;

CLIParser::CLIParser(int argc, char **argv)
    : _cli_options{ocarina::fs::path{argv[0]}.filename().string()} {
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

}// namespace vision