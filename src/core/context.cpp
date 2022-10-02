//
// Created by Zero on 05/10/2022.
//

#include "context.h"

namespace vision {

Context::Context(int argc, char **argv, ocarina::string_view cache_dir)
    : Super(fs::path(argv[0]).parent_path(), cache_dir),
      _cli_parser(argc, argv) {
}
}