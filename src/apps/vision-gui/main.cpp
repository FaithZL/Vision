//
// Created by Zero on 02/09/2022.
//

#include "application.h"

using namespace ocarina;
using namespace vision;

int main(int argc, char *argv[]) {
    fs::path runtime_dir = fs::path(argv[0]).parent_path();
    fs::current_path(runtime_dir);
    return App(argc, argv).run();
}