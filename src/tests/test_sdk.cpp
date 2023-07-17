//
// Created by Zero on 2023/7/17.
//

#include "sdk/vision.h"

using namespace vision::sdk;

void * module = nullptr;
visionCreator * creator = nullptr;
visionDeleter *deleter = nullptr;

template<typename T>
T *find_symbol(void *handle,const char *name_view) noexcept {
    auto symbol = GetProcAddress(reinterpret_cast<HMODULE>(handle), name_view);
    return reinterpret_cast<T *>(symbol);
}


int main() {
    module = LoadLibraryA("vision-renderer");
    creator = find_symbol<visionCreator>(module, "create");
    deleter = find_symbol<visionDeleter>(module, "destroy");

    auto vr = creator();

    return 0;
}