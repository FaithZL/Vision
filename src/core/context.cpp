//
// Created by Zero on 05/10/2022.
//

#include "context.h"
#include "descriptions/scene_desc.h"

namespace vision {

Context::Context(int argc, char **argv, ocarina::string_view cache_dir)
    : Super(fs::path(argv[0]).parent_path(), cache_dir),
      _cli_parser(argc, argv) {
}

SceneDesc Context::parse_file() const noexcept {
    return SceneDesc::from_json(cli_parser().scene_file());
}

fs::path Context::scene_directory() const noexcept {
    return cli_parser().scene_file().parent_path();
}

}// namespace vision