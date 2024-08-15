//
// Created by Zero on 2024/8/13.
//

#include "hotfix/rules_parser.h"

namespace vision::inline hotfix {

class NinjaParser : public BuildRulesParser {
public:
    void parse(const std::string &content) override;
};

void NinjaParser::parse(const std::string &content) {


}

}// namespace vision::inline hotfix