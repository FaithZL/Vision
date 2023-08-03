//
// Created by Zero on 2023/7/17.
//

#include "sdk/vision.h"
#include <filesystem>

using namespace vision::sdk;

int main(int argc, char *argv[]) {
    HMODULE module;
    auto vr = create_vision(module);
    vr->init_pipeline(std::filesystem::path(argv[0]).parent_path().string().c_str());
    vr->init_scene();


    return 0;
}