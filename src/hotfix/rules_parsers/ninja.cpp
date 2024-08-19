//
// Created by Zero on 2024/8/13.
//

#include "core/vs_header.h"
#include "hotfix/build_rules.h"

namespace vision::inline hotfix {

class NinjaParser : public BuildRules {
public:
    NinjaParser();
    void process_lines(string_view *lines);
    void parse(const std::string &content) override;
};

void NinjaParser::process_lines(std::string_view *lines) {
    string_view line = lines[0];

    auto find_dst = [&]{
        constexpr auto obj_token = ".cpp.obj";
        constexpr auto build_token = "build ";
        auto index = line.find(obj_token, 0);
        CompileOptions options;
        uint size_obj_token = strlen(obj_token);
        uint size_build_token = strlen(build_token);
        return line.substr(size_build_token, index + size_obj_token - size_build_token);
    };

    CompileOptions options;
    options.dst = find_dst();
    compiles_.push_back(options);
}

NinjaParser::NinjaParser() {
    std::ifstream fst;
    fs::path fn = fs::current_path().parent_path() / "build.ninja";
    string content = from_file(fn);
    parse(content);
}

void NinjaParser::parse(const std::string &content) {
    auto lines = string_split(content, '\n');
//    for (int i = 0; i < lines.size(); ++i) {
//        string_view& line = lines[i];
//        string_view& next_line = lines[i + 1];
//        if (line.starts_with("build") && next_line.starts_with("  DEFINES")) {
//            process_lines(addressof(line));
//        }
//    }
}

}// namespace vision::inline hotfix

VS_EXPORT_API vision::hotfix::BuildRules *create() {
    return ocarina::new_with_allocator<vision::hotfix::NinjaParser>();
}

VS_EXPORT_API void destroy(vision::hotfix::BuildRules *obj) {
    ocarina::delete_with_allocator(obj);
}