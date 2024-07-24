//
// Created by Zero on 2024/8/2.
//

#include "core/stl.h"

namespace vision::inline hotfix {
using namespace ocarina;

class CompileOption {

};

class Compiler {
};

class BuildTool {
private:
    Compiler compiler_;

public:
    BuildTool() = default;
    void clear() noexcept;
    void build(vector<fs::path> &&paths) noexcept;
};

}// namespace vision::inline hotfix