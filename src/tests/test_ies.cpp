//
// Created by Zero on 2023/8/21.
//

#include "base/illumination/ies.h"
#include "core/string_util.h"

using namespace ocarina;

int main() {

    auto path = "E:\\work\\renderer\\Vision\\res\\ies\\6.ies";

    string ies = from_file(path);
    vision::IESFile ies_file;

    ies_file.load(ies);

    return 0;
}