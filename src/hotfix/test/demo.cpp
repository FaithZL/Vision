//
// Created by Zero on 2024/8/21.
//

#include "demo.h"
#include "core/vs_header.h"
#include "hotfix/hotfix.h"

namespace vision::inline hotfix {

IObjectConstructor *Demo::constructor() const noexcept {
    std::cout << "wocaodfa--------------------" << std::endl;
    return nullptr;
}

void Demo::serialize(vision::Serializer *serializer) const noexcept {
}

void Demo::deserialize(vision::Serializer *serializer) const noexcept {
}

}// namespace vision::inline hotfix

VS_EXPORT_API vision::hotfix::Demo *create() {
    std::cout << "wocaooo" << std::endl;
    return ocarina::new_with_allocator<vision::hotfix::Demo>();
}
