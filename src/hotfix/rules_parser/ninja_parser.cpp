//
// Created by Zero on 2024/8/13.
//

#include "core/vs_header.h"
#include "hotfix/build_rules.h"

namespace vision::inline hotfix {

class NinjaParser : public BuildRules {
public:
    void parse(const std::string &content) override;
};

void NinjaParser::parse(const std::string &content) {
}

}// namespace vision::inline hotfix

VS_EXPORT_API vision::hotfix::BuildRules *create() {
    return ocarina::new_with_allocator<vision::hotfix::NinjaParser>();
}

VS_EXPORT_API void destroy(vision::hotfix::BuildRules *obj) {
    ocarina::delete_with_allocator(obj);
}