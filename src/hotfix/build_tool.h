//
// Created by Zero on 2024/8/2.
//

#include "compiler.h"

namespace vision::inline hotfix {
using namespace ocarina;

class BuildTool {
private:
    UP<Compiler> compiler_{Compiler::create()};

public:
    BuildTool() = default;
    void clear() noexcept;
    void build_module(vector<fs::path> &&paths) noexcept;
};

}// namespace vision::inline hotfix